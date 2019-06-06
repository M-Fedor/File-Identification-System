#ifndef OutputDBConnection_h
#define OutputDBConnection_h

#define DIGEST_SIZE 65
#define NAME_SIZE 300
#define VERSION_SIZE 200

#include "Output.h"
#include "OutputOffline.h"
#include <cstring>
#include <mysql/mysql.h>
#include <sstream>
#include <vector>

/* Class implements reentrant communication with MariaDB/MySQL DBMS
in order to determine output and outputs data in output file */
class OutputDBConnection : public Output
{
public:
  OutputDBConnection(OutputOffline *out, const char *host, const char *user, const char *passwd,
                     const char *db, unsigned int port, const char *unixSock);
  ~OutputDBConnection();

  int init();
  int outputData(std::string &digest, std::string &name);

private:
  int formatData(std::string &digest, std::string &name, std::string &data);
  int getData(std::string &digest, std::string &name);
  void printErr(const char *errInfo);
  void resizeBuffers();
  void setBind(
      MYSQL_BIND &bind, enum enum_field_types field_type, void *param, size_t paramSize,
      size_t *paramLen, my_bool &isNull, my_bool &error, char &ind);

  char *fileDigest;
  char *fileName;
  char *fileVersion;
  const char *dbName;
  const char *getDigestFileNameStr;
  const char *hostName;
  const char *unixSocket;
  const char *userName;
  const char *userPasswd;
  int bufferSizeFactor;
  MYSQL *mysql;
  MYSQL_BIND bind[5];
  MYSQL_STMT *getDigestFileName;
  MYSQL_TIME fileCreated;
  MYSQL_TIME fileChanged;
  OutputOffline *fOutput;
  std::vector<my_bool> error;
  std::vector<my_bool> isNull;
  std::vector<size_t> paramLen;
  unsigned int portNum;
};

#endif