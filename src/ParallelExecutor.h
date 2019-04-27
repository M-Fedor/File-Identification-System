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
#include <queue>
#include <thread>

class ParallelExecutor
{
public:
  ParallelExecutor(InputFile *in, std::vector<Output *> outputInstList, const char *errFile);
  ParallelExecutor(InputScanner *in, std::vector<HashAlgorithm *> hashAlgInstList,
                   std::vector<Output *> outputInstList, const char *errFile);
  ~ParallelExecutor();

  int init();
  void setInterrupted();
  void validate();

private:
  typedef struct FileData
  {
    FileData(std::ifstream &fDesc, std::string dig, std::string name)
        : fDescriptor(&fDesc), digest(dig), pathName(name) {}
    FileData(const FileData &d)
        : fDescriptor(d.fDescriptor), digest(d.digest), pathName(d.pathName) {}

    std::ifstream *fDescriptor;
    std::string digest;
    std::string pathName;
  } FileData;

  FileData popSync();
  int inputNextFile(std::ifstream &fDescriptor, std::string &digest, std::string &pathName);
  void pushSync(FileData &data);
  bool qAlmostEmptyPred();
  bool qReadyPred();
  static void threadFnInFile(Output *out, ParallelExecutor *execInst);
  static void threadFnInScanner(HashAlgorithm *hashAlg, Output *out, ParallelExecutor *execInst);

  const char *errFileName;
  InputFile *inFile;
  InputScanner *inScanner;
  OutputOffline *fError;
  std::atomic<bool> done;
  std::atomic<bool> interrupted;
  std::condition_variable queueAlmostEmpty;
  std::condition_variable queueEmpty;
  std::mutex queueAccessMutex;
  std::mutex queueEmptyMutex;
  std::queue<FileData> dataQueue;
  std::vector<HashAlgorithm *> hashAlgInstList;
  std::vector<Output *> outputInstList;
  std::vector<std::thread> threadList;
  unsigned int nCores;
};

#endif