#ifndef ParallelExecutor_h
#define ParallelExecutor_h

#define BUFFER_SIZE 2000
#define MAX_QUEUE_LOAD_FACTOR 16
#define MIN_QUEUE_LOAD_FACTOR 2

#include "InputFile.h"
#include "InputScanner.h"
#include "OutputDBConnection.h"
#include "OutputOffline.h"
#include "SHA2.h"
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <queue>
#include <thread>

/* Class implements simple framework for parallel file checks and validation */
class ParallelExecutor
{
public:
  ParallelExecutor(std::shared_ptr<InputFile> in, std::vector<std::shared_ptr<Output>> &outputInstLists);
  ParallelExecutor(std::shared_ptr<InputScanner> in, std::vector<std::shared_ptr<HashAlgorithm>> &hashAlgInstList,
                   std::vector<std::shared_ptr<Output>> &outputInstList);
  ParallelExecutor(std::shared_ptr<InputScanner> in, std::vector<std::shared_ptr<HashAlgorithm>> &hashAlgInstList,
                   std::shared_ptr<OutputOffline> out);
  ~ParallelExecutor();

  int init();
  int setErrFile(const char *errFileName);
  void setVerbose();
  void validate();

private:
  // Structure contains data obtained from an input component
  typedef struct FileData
  {
    FileData() {}
    FileData(std::ifstream &fDesc, std::string dig, std::string name)
        : digest(dig), pathName(name) { fDescriptor = std::move(fDesc); }
    FileData(const FileData &d) = delete;
    FileData(FileData &&d) = default;

    FileData &operator=(const FileData &) = delete;
    FileData &operator=(FileData &&d) = default;

    std::ifstream fDescriptor;
    std::string digest;
    std::string pathName;
  } FileData;

  int inputNextFile(
      std::ifstream &fDescriptor, std::string &digest, std::string &pathName);
  int popSync(FileData &data);
  void printStatus(bool end);
  void pushSync(FileData &data);
  bool qAlmostEmptyPred();
  bool qReadyPred();
  static void signalHandler(int signal);
  static void synchronize(ParallelExecutor *execInst);
  static void threadFnInFile(Output *out, ParallelExecutor *execInst);
  static void threadFnInScanner(
      HashAlgorithm *hashAlg, Output *out, ParallelExecutor *execInst);

  bool verbose;
  OutputOffline fError;
  std::atomic<bool> done;
  static std::atomic<bool> interrupted;
  std::condition_variable queueAlmostEmpty;
  std::condition_variable queueReady;
  std::mutex queueAccessMutex;
  std::mutex queueEmptyMutex;
  std::queue<FileData> dataQueue;
  std::shared_ptr<InputFile> inFile;
  std::shared_ptr<InputScanner> inScanner;
  std::vector<std::shared_ptr<HashAlgorithm>> hashAlgInstList;
  std::vector<std::shared_ptr<Output>> outputInstList;
  std::vector<std::thread> threadList;
  unsigned int failedJobs;
  unsigned int loadedJobs;
  unsigned int nCores;
};

#endif
