#ifndef InputScanner_h
#define InputScanner_h

#include "Input.h"
#include <cstring>
#include <dirent.h>
#include <vector>

/* Class implements file-system recursive iterator */
class InputScanner : public Input
{
public:
  InputScanner(std::string rootDirectory);
  InputScanner(std::vector<std::string> rootDirectories);
  ~InputScanner();

  int init();
  int inputNextFile(std::ifstream &fDescriptor, std::string &pathName);

private:
  int findNextFDRec(std::ifstream &fDescriptor, std::string &pathName);

  std::vector<std::string> absolutePaths;
  std::vector<DIR *> directoryStreams;
  std::vector<std::string> rootDirectories;
};

#endif