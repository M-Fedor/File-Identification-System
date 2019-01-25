#ifndef InputScanner_h
#define InputScanner_h

#include "Input.h"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

class InputScanner : public Input
{
public:
  InputScanner(std::string rootDirectory);
  InputScanner(std::vector<std::string> rootDirectories);
  ~InputScanner();

  int getNextFD(std::string &pathName);
  int init();

private:
  int findNextFDRec(std::string &pathName);

  std::vector<std::string> absolutePaths;
  std::vector<DIR *> directoryStreams;
  std::vector<std::string> rootDirectories;
};

#endif