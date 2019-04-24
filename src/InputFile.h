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
  InputFile(char *name);
  ~InputFile();

  int init();
  std::string inputDigest();
  int inputNextFile(std::string &pathName);

private:
  char *fileDigest;
  char *fileName;
  char *srcFileName;
  std::ifstream fInput;
  std::string digest;
};

#endif