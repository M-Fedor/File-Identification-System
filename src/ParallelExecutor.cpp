#include "ParallelExecutor.h"

ParallelExecutor::ParallelExecutor(InputFile *in, std::vector<Output *> outputInstList, const char *errFile)
    : errFileName(errFile), inFile(in), inScanner(NULL), fError(NULL), done(false), interrupted(false)
{
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = outputInstList.size();
}

ParallelExecutor::ParallelExecutor(
    InputScanner *in, std::vector<HashAlgorithm *> hashAlgInstList,
    std::vector<Output *> outputInstList, const char *errFile)
    : errFileName(errFile), inFile(NULL), inScanner(in), fError(NULL), done(false), interrupted(false)
{
    this->hashAlgInstList.assign(hashAlgInstList.begin(), hashAlgInstList.end());
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = std::min(hashAlgInstList.size(), outputInstList.size());
}

ParallelExecutor::~ParallelExecutor()
{
    if (inScanner)
    {
        delete inScanner;
        for (HashAlgorithm *hash : hashAlgInstList)
            delete hash;
        while (!dataQueue.empty())
        {
            dataQueue.front().fDescriptor->close();
            dataQueue.pop();
        }
    }
    else
        delete inFile;
    for (Output *out : outputInstList)
        delete out;
    delete fError;
}

int ParallelExecutor::init()
{
    if (!nCores)
        return 1;
    if (inFile)
    {
        if (inScanner || inFile->init())
            return 1;
    }
    else if (inScanner)
    {
        if (inFile || inScanner->init())
            return 1;
    }
    else
        return 1;

    for (unsigned int i = 0; i < nCores; i++)
    {
        if (inScanner)
        {
            if (!hashAlgInstList[i] || !outputInstList[i])
                return 1;
            if (outputInstList[i]->init())
                return 1;
            threadList.push_back(std::thread(
                threadFnInScanner, hashAlgInstList[i], outputInstList[i], this));
        }
        else if (inFile)
        {
            if (!outputInstList[i])
                return 1;
            if (outputInstList[i]->init())
                return 1;
            threadList.push_back(std::thread(
                threadFnInFile, outputInstList[i], this));
        }
    }

    fError = new OutputOffline(errFileName);
    if (fError->init())
        return 1;

    return 0;
}

int ParallelExecutor::inputNextFile(
    std::ifstream &fDescriptor, std::string &digest, std::string &pathName)
{
    int rc;
    if (inFile)
    {
        rc = inFile->inputNextFile(pathName);
        digest = inFile->inputDigest();
    }
    else
        rc = inScanner->inputNextFile(fDescriptor, pathName);

    return rc;
}

ParallelExecutor::FileData ParallelExecutor::popSync()
{
    std::unique_lock<std::mutex> lock(queueAccessMutex);
    FileData data = dataQueue.front();
    dataQueue.pop();
    return data;
}

void ParallelExecutor::pushSync(FileData &data)
{
    std::unique_lock<std::mutex> lock(queueAccessMutex);
    dataQueue.push(data);
}

bool ParallelExecutor::qAlmostEmptyPred() { return dataQueue.size() <
                                                   MIN_QUEUE_LOAD_FACTOR * nCores; }

bool ParallelExecutor::qReadyPred() { return dataQueue.size() >=
                                             MAX_QUEUE_LOAD_FACTOR * nCores; }

void ParallelExecutor::setInterrupted() { interrupted.store(true); }

void ParallelExecutor::threadFnInFile(Output *out, ParallelExecutor *execInst)
{
    std::unique_lock<std::mutex> lock(execInst->queueAccessMutex);
    while (!execInst->qReadyPred())
        execInst->queueEmpty.wait(lock);
    lock.unlock();
    int rc;

    while (!execInst->interrupted)
    {
        if (execInst->dataQueue.empty())
        {
            if (execInst->done)
                break;
            else
            {
                lock.lock();
                while (execInst->qAlmostEmptyPred())
                    execInst->queueEmpty.wait(lock);
                lock.unlock();
            }
        }
        else if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();

        FileData data = execInst->popSync();
        do
            rc = out->outputData(data.digest, data.pathName);
        while (rc == 2);
        if (rc == -1)
            execInst->fError->outputData(data.digest, data.pathName);
    }
}

void ParallelExecutor::threadFnInScanner(
    HashAlgorithm *hashAlg, Output *out, ParallelExecutor *execInst)
{
    char *buffer = new char[BUFFER_SIZE];
    std::unique_lock<std::mutex> lock(execInst->queueAccessMutex);
    while (!execInst->qReadyPred())
        execInst->queueEmpty.wait(lock);
    lock.unlock();
    int rc;

    while (!execInst->interrupted)
    {
        if (execInst->dataQueue.empty())
        {
            if (execInst->done)
                break;
            else
            {
                lock.lock();
                while (execInst->qAlmostEmptyPred())
                    execInst->queueEmpty.wait(lock);
                lock.unlock();
            }
        }
        else if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();

        FileData data = execInst->popSync();
        while (data.fDescriptor->good())
        {
            data.fDescriptor->read(buffer, BUFFER_SIZE);
            hashAlg->inputData(buffer, BUFFER_SIZE);
        }
        data.fDescriptor->close();
        if (data.fDescriptor->fail())
            continue;

        data.digest = hashAlg->hashData();
        do
            rc = out->outputData(data.digest, data.pathName);
        while (rc == 2);
        if (rc == -1)
            execInst->fError->outputData(data.digest, data.pathName);
    }
    delete[] buffer;
}

void ParallelExecutor::validate()
{
    std::string digest;
    std::string pathName;
    std::unique_lock<std::mutex> lock(queueEmptyMutex);

    while (!interrupted && !done)
    {
        while (dataQueue.size() < 16 * nCores)
        {
            std::ifstream fDescriptor;
            if (inputNextFile(fDescriptor, digest, pathName) == -1)
            {
                done.store(true);
                break;
            }
            FileData data(fDescriptor, digest, pathName);
            pushSync(data);
        }
        queueEmpty.notify_all();
        while (!qAlmostEmptyPred())
            queueAlmostEmpty.wait(lock);
    }

    for (unsigned int i = 0; i < nCores; i++)
        threadList[i].join();
}