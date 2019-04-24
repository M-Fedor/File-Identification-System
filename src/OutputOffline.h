#ifndef OutputOffline_h
#define OutputOffline_h

#include "Output.h"
#include <mutex>

/* Class implements output of computed data, when no connection
to DBMS is available */
class OutputOffline : public Output
{
public:
  OutputOffline(char *fileName);
  ~OutputOffline();

  int init();
  int outputData(std::string data);
  int outputData(std::string digest, std::string name);

private:
  char *fileName;
  std::mutex mutex;
  std::ofstream fOutput;
};

#endif