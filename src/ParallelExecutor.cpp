#include "ParallelExecutor.h"

ParallelExecutor::ParallelExecutor(
    InputFile *in, std::vector<Output *> &outputInstList, const char *errFile)
    : errFileName(errFile), inFile(in), inScanner(NULL), fError(NULL), done(false), interrupted(false)
{
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = outputInstList.size();
}

ParallelExecutor::ParallelExecutor(
    InputScanner *in, std::vector<HashAlgorithm *> &hashAlgInstList,
    std::vector<Output *> &outputInstList, const char *errFile)
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

    std::unique_lock<std::mutex> lock(queueAccessMutex);
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
            queueAlmostEmpty.wait(lock);
        }
        else if (inFile)
        {
            if (!outputInstList[i])
                return 1;
            if (outputInstList[i]->init())
                return 1;
            threadList.push_back(std::thread(
                threadFnInFile, outputInstList[i], this));
            queueAlmostEmpty.wait(lock);
        }
    }

    if (errFileName)
    {
        fError = new OutputOffline(errFileName);
        if (fError->init())
            return 1;
    }

    return 0;
}

int ParallelExecutor::inputNextFile(
    std::ifstream *fDescriptor, std::string &digest, std::string &pathName)
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

int ParallelExecutor::popSync(FileData &data)
{
    std::unique_lock<std::mutex> lock(queueAccessMutex);
    while (dataQueue.empty() && !done)
    {
        std::cout << "queue empty\n";
        while (qAlmostEmptyPred() && !done)
            queueReady.wait(lock);
    }
    if (dataQueue.empty() && done)
        return 1;

    data = dataQueue.front();
    dataQueue.pop();
    return 0;
}

void ParallelExecutor::pushSync(FileData &data)
{
    std::lock_guard<std::mutex> lock(queueAccessMutex);
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
    execInst->queueAlmostEmpty.notify_all();
    while (!execInst->qReadyPred() && !execInst->done)
        execInst->queueReady.wait(lock);
    lock.unlock();
    int rc;

    while (!execInst->interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();
        FileData data;
        if (execInst->popSync(data))
            break;
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
    execInst->queueAlmostEmpty.notify_all();
    while (!execInst->qReadyPred() && !execInst->done)
        execInst->queueReady.wait(lock);
    lock.unlock();
    int rc;

    while (!execInst->interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();

        FileData data;
        if (execInst->popSync(data))
            break;
        while (data.fDescriptor->good())
        {
            data.fDescriptor->read(buffer, BUFFER_SIZE);
            hashAlg->inputData(buffer, BUFFER_SIZE);
        }
        data.fDescriptor->close();
        if (data.fDescriptor->fail() && !data.fDescriptor->eof())
            continue;

        data.digest = hashAlg->hashData();
        do
            rc = out->outputData(data.digest, data.pathName);
        while (rc == 2);
    }
    delete[] buffer;
}

void ParallelExecutor::validate()
{
    std::ifstream *fDescriptor;
    std::string digest;
    std::string pathName;
    std::unique_lock<std::mutex> lock(queueEmptyMutex);

    while (!interrupted && !done)
    {
        while (dataQueue.size() < 16 * nCores)
        {
            fDescriptor = new std::ifstream();
            if (inputNextFile(fDescriptor, digest, pathName) == -1)
            {
                std::unique_lock<std::mutex> lockAccess(queueAccessMutex);
                std::cout << "done\n";
                done.store(true);
                break;
            }
            FileData data(fDescriptor, digest, pathName);
            pushSync(data);
        }
        queueReady.notify_all();
        while (!qAlmostEmptyPred() && !done)
            queueAlmostEmpty.wait(lock);
    }

    for (unsigned int i = 0; i < nCores; i++)
        threadList[i].join();
}