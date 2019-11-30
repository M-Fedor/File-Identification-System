#ifndef DBConnection_h
#define DBConnection_h

#include "Utils.h"
#include <cstring>
#include <memory>
#include <vector>

#if defined(__linux__)
#include <mysql/mysql.h>
#elif defined(_WIN32)
#include <mysql.h>
#endif

class DBConnection
{
public:
    DBConnection();
    DBConnection(const char *host, const char *user, const char *passwd,
                 const char *db, unsigned int port, const char *unixSock);
    ~DBConnection();
    DBConnection(const DBConnection &c) = delete;
    DBConnection(DBConnection &&c) = default;

    DBConnection &operator=(const DBConnection &) = delete;
    DBConnection &operator=(DBConnection &&c) = default;

    int bindResults(
        MYSQL_TIME fileCreated, MYSQL_TIME fileChanged, MYSQL_TIME fileRegistered,
        char *fileDigest, char *fileName, char *fileVersion, char *osCombination, char *swPackage);
    int executeSelect(std::string &digest, std::string &name);
    int fetchData();
    int init(const char *query);
    void setSize(int nameSize, int digestSize, int versionSize);

private:
    int printErr(const char *errInfo);
    void setBind(
        MYSQL_BIND &bind, enum enum_field_types field_type, void *param, size_t paramSize,
        size_t *paramLen, my_bool &isNull, my_bool &error, char &ind);

    const char *dbName;
    const char *hostName;
    const char *unixSocket;
    const char *userName;
    const char *userPasswd;
    int digestSize;
    int nameSize;
    int versionSize;
    MYSQL *mysql;
    MYSQL_BIND bind[8];
    MYSQL_STMT *stmt;
    std::vector<my_bool> error;
    std::vector<my_bool> isNull;
    std::vector<size_t> paramLen;
    unsigned int portNum;
};

#endif
