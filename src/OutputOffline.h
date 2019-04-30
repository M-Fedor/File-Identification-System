#ifndef OutputOffline_h
#define OutputOffline_h

#include "Output.h"
#include <fstream>
#include <mutex>

/* Class implements output of computed data, when no connection
to DBMS is available */
class OutputOffline : public Output
{
public:
  OutputOffline(const char *fileName);
  ~OutputOffline();

  int init();
  int outputData(std::string &data);
  int outputData(std::string &digest, std::string &name);

private:
  const char *fileName;
  static std::mutex mutex;
  std::ofstream fOutput;
};

#endif