#ifndef OutputValidateDB_h
#define OutputValidateDB_h

#define DIGEST_SIZE 65
#define NAME_SIZE 300
#define VERSION_SIZE 200

#include "DBConnection.h"
#include "Output.h"
#include "OutputOffline.h"

class OutputValidateDB : public Output
{
public:
  OutputValidateDB(DBConnection &conn, std::shared_ptr<OutputOffline> out);
  ~OutputValidateDB();

  int init();
  int outputData(std::string &digest, std::string &name);

private:
  int formatData(std::string &digest, std::string &name, std::string &data);
  void resizeBuffers();

  DBConnection connection;
  int bufferSizeFactor;
  MYSQL_TIME fileCreated;
  MYSQL_TIME fileChanged;
  MYSQL_TIME fileRegistered;
  std::shared_ptr<OutputOffline> fOutput;
  std::unique_ptr<char[]> fileDigest;
  std::unique_ptr<char[]> fileName;
  std::unique_ptr<char[]> fileVersion;
  std::unique_ptr<char[]> osCombination;
  std::unique_ptr<char[]> swPackage;
};

#endif
