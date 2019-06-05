#ifndef InputFile_h
#define InputFile_h

#define DIGEST_SIZE 65
#define NAME_SIZE 300

#include "Input.h"

/* Class implements input from source file containing
list of files represented by their absolute paths in file-system and 
their corresponding precomputed unique identifiers */
class InputFile : public Input
{
public:
  InputFile(const char *name, const char *pattern = NULL);
  ~InputFile();

  int init();
  int inputNextFile(std::string &digest, std::string &pathName);

private:
  char *fileDigest;
  char *fileName;
  const char *srcFileName;
  std::ifstream fInput;
};

#endif