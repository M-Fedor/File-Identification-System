#ifndef OutputOffline_h
#define OutputOffline_h

#include "Output.h"
#include <cstring>

class OutputOffline : public Output
{
public:
  OutputOffline();
  ~OutputOffline();

  int init();
  int outputData(std::string digest, std::string name);
};

#endif