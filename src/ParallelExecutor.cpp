#include "ParallelExecutor.h"

std::atomic<bool> ParallelExecutor::interrupted(false);

/* Constructor; set the input component to "InputFile" and set any output component.
Number of threads is determined by number of output component instances provided */
ParallelExecutor::ParallelExecutor(
    std::shared_ptr<InputFile> in, std::vector<std::shared_ptr<Output>> &outputInstList)
    : verbose(false), done(false), inFile(in), inScanner(NULL), failedJobs(0), loadedJobs(0)
{
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = outputInstList.size();
}

/* Constructor; set input component to "InputScanner" and set any hash algorithm and output components.
Number of threads is determined by minimum of number of output component and hash algorithm instances provided */
ParallelExecutor::ParallelExecutor(
    std::shared_ptr<InputScanner> in, std::vector<std::shared_ptr<HashAlgorithm>> &hashAlgInstList,
    std::vector<std::shared_ptr<Output>> &outputInstList)
    : verbose(false), done(false), inFile(NULL), inScanner(in), failedJobs(0), loadedJobs(0)
{
    this->hashAlgInstList.assign(hashAlgInstList.begin(), hashAlgInstList.end());
    this->outputInstList.assign(outputInstList.begin(), outputInstList.end());
    nCores = std::min(hashAlgInstList.size(), outputInstList.size());
}

/* Constructor; set input component to "InputScanner", set any hashAlgorithm and single OutputOffline instance. 
This is preferred way to utilize "OutputOffline" output component. */
ParallelExecutor::ParallelExecutor(
    std::shared_ptr<InputScanner> in, std::vector<std::shared_ptr<HashAlgorithm>> &hashAlgInstList,
    std::shared_ptr<OutputOffline> out)
    : verbose(false), done(false), inFile(NULL), inScanner(in), failedJobs(0), loadedJobs(0)
{
    this->hashAlgInstList.assign(hashAlgInstList.begin(), hashAlgInstList.end());
    nCores = hashAlgInstList.size();
    for (unsigned int i = 0; i < nCores; i++)
        outputInstList.push_back(out);
}

/* Destructor */
ParallelExecutor::~ParallelExecutor() {}

/* Loads input using input component, controls interruption 
and termination of worker threads */
void ParallelExecutor::execute()
{
    std::string digest;
    std::string pathName;
    std::unique_lock<std::mutex> lock(queueEmptyMutex);

    while (!interrupted && !done)
    {
        while (!qReadyPred())
        {
            std::ifstream fDescriptor;
            if (inputNextFile(fDescriptor, digest, pathName) == UNDEFINED)
            {
                std::unique_lock<std::mutex> lockAccess(queueAccessMutex);
                done.store(true);
                break;
            }
            FileData data(fDescriptor, digest, pathName);
            pushSync(data);
            loadedJobs++;
        }
        if (verbose)
            printStatus(false);
        queueReady.notify_all();
        while (!qAlmostEmptyPred() && !done && !interrupted)
            queueAlmostEmpty.wait(lock);
    }

    for (auto &thread : threadList)
        thread.join();
    if (verbose)
        printStatus(true);
}

/* Validate input parameters and initialize multithreaded environment */
int ParallelExecutor::init()
{
    if (!nCores)
        return FAIL;
    if (inFile)
    {
        if (inFile->init())
            return FAIL;
    }
    else if (inScanner)
    {
        if (inScanner->init())
            return FAIL;
    }
    else
        return FAIL;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::unique_lock<std::mutex> lock(queueAccessMutex);
    for (unsigned int i = 0; i < nCores; i++)
    {
        if (!outputInstList[i])
            return FAIL;
        if (outputInstList[i]->init())
            return FAIL;
        if (inScanner)
        {
            if (!hashAlgInstList[i])
                return FAIL;
            threadList.emplace_back(threadFnInScanner, hashAlgInstList[i].get(),
                                    outputInstList[i].get(), this);
            queueAlmostEmpty.wait(lock);
        }
        else if (inFile)
        {
            threadList.emplace_back(threadFnInFile, outputInstList[i].get(), this);
            queueAlmostEmpty.wait(lock);
        }
    }

    return OK;
}

/* Input information about next file based on input component currently being used */
int ParallelExecutor::inputNextFile(
    std::ifstream &fDescriptor, std::string &digest, std::string &pathName)
{
    return (inFile) ? inFile->inputNextFile(digest, pathName)
                    : inScanner->inputNextFile(fDescriptor, pathName);
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
        return FAIL;

    data = std::move(dataQueue.front());
    dataQueue.pop();
    return OK;
}

void ParallelExecutor::printStatus(bool end)
{
    std::cout << "Loaded files:      [";
    printGreen(std::to_string(loadedJobs).data());
    std::cout << "]\nProcessing errors: [";
    printRed(std::to_string(failedJobs).data());
    std::cout << "]";
    if (!end)
        resetCursor();
    else
        std::cout << "\n\n";
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

int ParallelExecutor::setErrFile(const char *errFileName)
{
    fError = OutputOffline(errFileName);
    if (fError.init())
        return FAIL;
    return OK;
}

void ParallelExecutor::setVerbose() { verbose = true; }

void ParallelExecutor::signalHandler(int /* signal */) { interrupted.store(true); }

/* Synchronize with other threads up to a point when ready for data processing */
void ParallelExecutor::synchronize(ParallelExecutor *execInst)
{
    std::unique_lock<std::mutex> lock(execInst->queueAccessMutex);
    execInst->queueAlmostEmpty.notify_all();
    while (!execInst->qReadyPred() && !execInst->done)
        execInst->queueReady.wait(lock);
}

/* Method being executed by worker thread when input component is set to "InputFile"; 
Obtains FileData structures from shared queue and processes them using output component */
void ParallelExecutor::threadFnInFile(Output *out, ParallelExecutor *execInst)
{
    synchronize(execInst);
    int rc;

    while (!interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();
        FileData data;
        if (execInst->popSync(data))
            break;
        rc = out->outputData(data.digest, data.pathName);
        if (rc == FAIL)
        {
            execInst->fError.outputData(data.digest, data.pathName);
            execInst->failedJobs++;
        }
    }
    execInst->queueAlmostEmpty.notify_all();
}

/* Method being executed by worker thread when input component is set to "InputScanner"
Obtains FileData structures from shared queue, computes corresponding unique identifiers 
and processes them using output component */
void ParallelExecutor::threadFnInScanner(
    HashAlgorithm *hashAlg, Output *out, ParallelExecutor *execInst)
{
    std::unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);
    int rc;
    synchronize(execInst);

    while (!interrupted)
    {
        if (execInst->qAlmostEmptyPred())
            execInst->queueAlmostEmpty.notify_all();

        FileData data;
        if (execInst->popSync(data))
            break;
        while (data.fDescriptor.good())
        {
            data.fDescriptor.read(buffer.get(), BUFFER_SIZE);
            hashAlg->inputData(buffer.get(), data.fDescriptor.gcount());
        }
        if (data.fDescriptor.eof())
        {
            data.digest = hashAlg->hashData();
            rc = out->outputData(data.digest, data.pathName);
            if (rc == FAIL)
            {
                execInst->fError.outputData(data.digest, data.pathName);
                execInst->failedJobs++;
            }
        }
    }
    execInst->queueAlmostEmpty.notify_all();
}
