#ifndef Output_h
#define Output_h

#include <string>

class Output
{
public:
  Output();
  virtual ~Output();

  virtual int init();
  virtual int outputData(std::string digest, std::string name);
};

#endif