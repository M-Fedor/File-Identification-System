#ifndef OutputOffline_h
#define OutputOffline_h

#include "Output.h"

/* Class implements output of computed data, when no connection
to DBMS is available */
class OutputOffline : public Output
{
public:
  OutputOffline();
  ~OutputOffline();

  int init();
  int outputData(std::string digest, std::string name);
};

#endif