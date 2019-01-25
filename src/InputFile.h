#ifndef InputFile_h
#define InputFile_h

#define DIGEST_SIZE 64
#define NAME_SIZE 100

#include "Input.h"
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

class InputFile : public Input
{
public:
  InputFile();
  ~InputFile();

  std::string getNextDigest();
  int getNextFD(std::string &pathName);
  int init();

private:
  char *fileDigest;
  char *fileName;
  FILE *fRead;
  long int currentPos;
  std::string digest;
};

#endif