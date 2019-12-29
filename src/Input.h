#ifndef Input_h
#define Input_h

#include "Utils.h"
#include <fstream>
#include <regex>

/* Interface for data input component */
class Input
{
public:
  Input();
  virtual ~Input();

  /* Initialize input component and report any failures */
  virtual int init();

  /* Increment file-system iterator, fill opened file descriptor
  and absolute path of the file in respective parameters */
  virtual int inputNextFile(std::ifstream &fDescriptor, std::string &pathName);

protected:
  std::regex regex;
};

#endif
