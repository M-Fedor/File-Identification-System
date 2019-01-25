#ifndef InputFile_h
#define InputFile_h

#define DIGEST_SIZE 64
#define NAME_SIZE 300

#include "Input.h"
#include <cstring>

class InputFile : public Input
{
public:
  InputFile();
  ~InputFile();

  int init();
  std::string inputDigest();
  int inputNextFile(std::string &pathName);

private:
  char *fileDigest;
  char *fileName;
  FILE *fRead;
  std::string digest;
};

#endif