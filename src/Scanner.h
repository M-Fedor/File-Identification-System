#ifndef Scanner_h
#define Scanner_h

#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

class Scanner
{
public:
  Scanner(std::string rootDirectory);
  Scanner(std::vector<std::string> rootDirectories);
  ~Scanner();

  int findNextFDRec();
  int getNextFD();
  int init();

private:
  std::vector<std::string> absolutePaths;
  std::vector<DIR *> directoryStreams;
  std::vector<std::string> rootDirectories;
};

#endif