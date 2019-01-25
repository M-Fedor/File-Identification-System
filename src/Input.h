#ifndef Input_h
#define Input_h

#include <string>

class Input
{
public:
  Input();
  virtual ~Input();

  virtual int init();
  virtual int inputNextFile(std::string &pathName);
};

#endif