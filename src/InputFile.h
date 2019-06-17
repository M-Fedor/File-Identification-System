#ifndef InputFile_h
#define InputFile_h

#include "Input.h"

/* Class implements input from source file containing
list of files represented by their absolute paths in file-system and 
their corresponding precomputed unique identifiers */
class InputFile : public Input
{
public:
  InputFile(const char *name, const char *pattern = ".*");
  ~InputFile();

  int init();
  int inputNextFile(std::string &digest, std::string &pathName);

private:
  const char *srcFileName;
  std::ifstream fInput;
};

#endif