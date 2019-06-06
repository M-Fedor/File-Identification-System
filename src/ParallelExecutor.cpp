#include "ParallelExecutor.h"

/* Constructor; set the input component to "InputFile" and set any output component.
Number of threads is determined by number of output component instances provided */
ParallelExecutor::ParallelExecutor(
    InputFile *in, std::vector<Output *> &outputInstList, const char *errFile)
    : errFileName(errFile), inFile(in), inScanner(NULL), done(false), interrupted(false)
{
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = outputInstList.size();
}

/* Constructor; set input component to "InputScanner" and set any hash algorithm and output components.
Number of threads is determined by minimum of number of output component and hash algorithm instances provided */
ParallelExecutor::ParallelExecutor(InputScanner *in, std::vector<HashAlgorithm *> &hashAlgInstList,
                                   std::vector<Output *> &outputInstList, const char *errFile)
    : errFileName(errFile), inFile(NULL), inScanner(in), done(false), interrupted(false)
{
    this->hashAlgInstList.assign(hashAlgInstList.begin(), hashAlgInstList.end());
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = std::min(hashAlgInstList.size(), outputInstList.size());
}

/* Constructor; set input component to "InputScanner", set any hashAlgorithm and single OutputOffline instance. 
This is preferred way to utilize "OutputOffline" output component. */
ParallelExecutor::ParallelExecutor(InputScanner *in, std::vector<HashAlgorithm *> &hashAlgInstList,
                                   OutputOffline *out, const char *errFile)
    : errFileName(errFile), inFile(NULL), inScanner(in), done(false), interrupted(false)
{
    this->hashAlgInstList.assign(hashAlgInstList.begin(), hashAlgInstList.end());
    nCores = hashAlgInstList.size();
    for (unsigned int i = 0; i < nCores; i++)
        outputInstList.push_back(out);
}

/* Destructor */
ParallelExecutor::~ParallelExecutor() {}

/* Validate input parameters and initialize multithreaded environment */
int ParallelExecutor::init()
{
    if (!nCores)
        return 1;
    if (inFile)
    {
        if (inFile->init())
            return 1;
    }
    else if (inScanner)
    {
        if (inScanner->init())
            return 1;
    }
    else
        return 1;

    if (errFileName)
    {
        fError = OutputOffline(errFileName);
        if (fError.init())
            return 1;
    }

    std::unique_lock<std::mutex> lock(queueAccessMutex);
    for (unsigned int i = 0; i < nCores; i++)
    {
        if (!outputInstList[i])
            return 1;
        if (outputInstList[i]->init())
            return 1;
        if (inScanner)
        {
            if (!hashAlgInstList[i])
                return 1;
            threadList.emplace_back(
                threadFnInScanner, hashAlgInstList[i], outputInstList[i], this);
            queueAlmostEmpty.wait(lock);
        }
        else if (inFile)
        {
            threadList.emplace_back(threadFnInFile, outputInstList[i], this);
            queueAlmostEmpty.wait(lock);
        }
    }

    return 0;
}

/* Input information about next file based on input component currently being used */
int ParallelExecutor::inputNextFile(
    std::ifstream &fDescriptor, std::string &digest, std::string &pathName)
{
    int rc;
    if (inFile)
        rc = inFile->inputNextFile(digest, pathName);
    else
        rc = inScanner->inputNextFile(fDescriptor, pathName);
    return rc;
}

/* Pop FileData structure from shared queue thread-safely; 
wait if queue is empty */
int ParallelExecutor::popSync(FileData &data)
{
    std::unique_lock<std::mutex> lock(queueAccessMutex);
    while (dataQueue.empty() && !done)
    {
        while (qAlmostEmptyPred() && !done)
            queueReady.wait(lock);
    }
    if (dataQueue.empty() && done)
        return 1;

    data = std::move(dataQueue.front());
    dataQueue.pop();
    return 0;
}

/* Push FileData structure to shared queue thread-safely */
void ParallelExecutor::pushSync(FileData &data)
{
    std::lock_guard<std::mutex> lock(queueAccessMutex);
    dataQueue.push(std::move(data));
}

bool ParallelExecutor::qAlmostEmptyPred() { return dataQueue.size() <
                                                   MIN_QUEUE_LOAD_FACTOR * nCores; }

bool ParallelExecutor::qReadyPred() { return dataQueue.size() >=
                                             MAX_QUEUE_LOAD_FACTOR * nCores; }

void ParallelExecutor::setInterrupted() { interrupted.store(true); }

/* Synchronize with other threads up to a point when ready for data processing */
void ParallelExecutor::synchronize(ParallelExecutor *execInst)
{
    std::unique_lock<std::mutex> lock(execInst->queueAccessMutex);
    execInst->queueAlmostEmpty.notify_all();
    while (!execInst->qReadyPred() && !execInst->done)
        execInst->queueReady.wait(lock);
    lock.unlock();
}

/* Method being executed by worker thread when input component is set to "InputFile"; 
Obtains FileData structures from shared queue and processes them using output component */
void ParallelExecutor::threadFnInFile(Output *out, ParallelExecutor *execInst)
{
    synchronize(execInst);
    int rc;

    while (!execInst->interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();
        FileData data;
        if (execInst->popSync(data))
            break;
        rc = out->outputData(data.digest, data.pathName);
        if (rc == 1)
            execInst->fError.outputData(data.digest, data.pathName);
    }
}

/* Method being executed by worker thread when input component is set to "InputScanner"
Obtains FileData structures from shared queue, computes corresponding unique identifiers 
and processes them using output component */
void ParallelExecutor::threadFnInScanner(
    HashAlgorithm *hashAlg, Output *out, ParallelExecutor *execInst)
{
    char *buffer = new char[BUFFER_SIZE];
    int rc;
    synchronize(execInst);

    while (!execInst->interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();

        FileData data;
        if (execInst->popSync(data))
            break;
        while (data.fDescriptor.good())
        {
            data.fDescriptor.read(buffer, BUFFER_SIZE);
            hashAlg->inputData(buffer, data.fDescriptor.gcount());
        }
        if (data.fDescriptor.eof())
        {
            data.digest = hashAlg->hashData();
            rc = out->outputData(data.digest, data.pathName);
            if (rc == 1)
                execInst->fError.outputData(data.digest, data.pathName);
        }
    }
    delete[] buffer;
}

/* Loads input using input component, controls interruption 
and termination of worker threads */
void ParallelExecutor::validate()
{
    std::string digest;
    std::string pathName;
    std::unique_lock<std::mutex> lock(queueEmptyMutex);

    while (!interrupted && !done)
    {
        while (!qReadyPred())
        {
            std::ifstream fDescriptor;
            if (inputNextFile(fDescriptor, digest, pathName) == -1)
            {
                std::unique_lock<std::mutex> lockAccess(queueAccessMutex);
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