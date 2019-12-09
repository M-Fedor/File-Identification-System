#ifndef OutputValidateDB_h
#define OutputValidateDB_h

#define DIGEST_SIZE 65
#define NAME_SIZE 300
#define TIMESTAMPS_INFO_ATTR_COUNT 3
#define VERSION_INFO_ATTR_COUNT 6
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
  void evaluateData(std::string &digest, std::string &name, std::stringstream &str);
  int formatData(std::string &digest, std::string &name, std::string &data);
  int getData(std::string &digest, std::string &name);
  void resizeBuffers();

  DBConnection connection;
  int bufferSizeFactor;
  std::shared_ptr<OutputOffline> fOutput;
  std::unique_ptr<char[]> fileDigest;
  std::unique_ptr<char[]> fileName;
  std::unique_ptr<char[]> fileType;
  std::vector<MYSQL_TIME> timestamps;
  std::vector<std::shared_ptr<char[]>> versionInfo;
};

#endif