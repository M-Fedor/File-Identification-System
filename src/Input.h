#ifndef Input_h
#define Input_h

#include "Utils.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

/* Interface for data input component */
class Input
{
public:
  Input();
  virtual ~Input();

  /* Initialize input component and report any failures */
  virtual int init();

  /* Increment file-system iterator, return opened file descriptor
  and fill absolute path of the file in pathName */
  virtual int inputNextFile(std::ifstream &fDescriptor, std::string &pathName);

protected:
  std::regex regex;
};

#endif
