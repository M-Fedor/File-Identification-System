#ifndef InputScanner_h
#define InputScanner_h

#include "Input.h"
#include <cstring>
#include <dirent.h>
#include <vector>

#if defined(__linux__)
#define DEFAULT_SEPARATOR "/"
#include <sys/stat.h>
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

  int init();
  int inputNextFile(std::ifstream &fDescriptor, std::string &pathName);

private:
  bool isDirectory(std::string &path);
  int findNextFDRec(std::ifstream &fDescriptor, std::string &pathName);
  void printErr(int errNUm, const std::ostringstream &errInfo);

  std::vector<std::string> absolutePaths;
  std::vector<DIR *> directoryStreams;
  std::vector<std::string> rootDirectories;

#if defined(__linux__)
  struct stat buffer;
#endif
};

#endif
