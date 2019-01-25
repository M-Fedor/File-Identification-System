#ifndef Input_h
#define Input_h

#include <string>

class Input
{
public:
  Input();
  virtual ~Input();

  virtual int getNextFD(std::string &pathName);
  virtual int init();
};

#endif