#ifndef InputScanner_h
#define InputScanner_h

#include "Input.h"
#include <cstring>
#include <dirent.h>
#include <vector>

#if defined(__linux__)
#define DEFAULT_SEPARATOR "/"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#elif defined(_WIN32)
#define DEFAULT_SEPARATOR "\\"
#include <fileapi.h>
#endif

/* Class implements file-system recursive iterator */
class InputScanner : public Input
{
public:
  InputScanner(std::string &rootDirectory, const char *pattern = ".*");
  InputScanner(std::vector<std::string> &rootDirectories, const char *pattern = ".*");
  ~InputScanner();

  void setEnableDataStreams(bool enable);
  int init();
  int inputNextFile(std::ifstream &fDescriptor, std::string &pathName);

private:
  int enumerateNextAlternateStream(std::ifstream &fDescriptor, std::string &pathName);
  int findNextFDRec(std::ifstream &fDescriptor, std::string &pathName);
  bool hasAlternateStreamDir(std::string &pathName);
  bool hasAlternateStreamFile(std::string &pathName);
  bool isDirectory(std::string &path);
  int loadFile(std::ifstream &fDescriptor, std::string &pathName);
  int printErr(int errNUm, const std::ostringstream &errInfo);

  bool enableDataStreams;
  bool hasNextAlternateStream;
  std::vector<std::string> absolutePaths;
  std::vector<DIR *> directoryStreams;
  std::vector<std::string> rootDirectories;

#if defined(__linux__)
  char *currentPosition;
  ssize_t attrSize;
  std::string currentPathName;
  std::unique_ptr<char[]> attr;
  struct stat buffer;
#elif defined(_WIN32)
  bool getFirstAlternateStream(std::string &pathName);
  bool getNextAlternateStream(std::string &pathName);

  HANDLE nextAlternateStream;
  std::wstring currentPathNameW;
  WIN32_FIND_STREAM_DATA streamData;
#endif
};

#endif
