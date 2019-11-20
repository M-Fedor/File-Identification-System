#ifndef OutputOffline_h
#define OutputOffline_h

#include "Output.h"
#include <fstream>
#include <mutex>

/* Class implements thread-safe output of computed data, when no connection
to DBMS is available; OS level synchronisation of multiple instances 
associated with the same file is NOT implemented!
Only SINGLE instance associated with the file should be used at the same time!
Otherwise, the order characters are writen to file is UNDEFINED.  */
class OutputOffline : public Output
{
public:
  OutputOffline(const char *fileName = NULL);
  ~OutputOffline();

  OutputOffline &operator=(OutputOffline &&o) = default;

  int init();
  int outputData(std::string &data);
  int outputData(std::string &digest, std::string &name);

private:
  const char *fileName;
  static std::mutex mutex;
  std::ofstream fOutput;
};

#endif
