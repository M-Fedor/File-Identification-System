#include "DBConnection.h"

DBConnection::DBConnection() {}

DBConnection::DBConnection(
    const char *host, const char *user, const char *passwd,
    const char *db, unsigned int port, const char *unixSock)
    : dbName(db), hostName(host), unixSocket(unixSock), userName(user), userPasswd(passwd),
      digestSize(0), nameSize(0), versionSize(0), mysql(NULL), stmt(NULL), portNum(port)
{
    // Initialize indicators for query execution
    error = std::vector<my_bool>(MAX_ATTR_COUNT, 0);
    isNull = std::vector<my_bool>(MAX_ATTR_COUNT, 0);
    paramLen = std::vector<size_t>(MAX_ATTR_COUNT, 0);

    memset(&bind, 0, sizeof(MYSQL_BIND) * MAX_ATTR_COUNT);
}

DBConnection::~DBConnection()
{
    if (stmt)
    {
        if (mysql_stmt_close(stmt))
            printErr("Close MySQL statement");
    }
    if (mysql)
        mysql_close(mysql);
}

int DBConnection::bindResults(char *fileName, std::vector<MYSQL_TIME> &timestamps, char *fileDigest,
                              char *fileType, std::vector<std::shared_ptr<char[]>> &versionInfo)
{
    char noneInd = STMT_INDICATOR_NONE;

    setBind(bind[0], MYSQL_TYPE_STRING, fileName, nameSize, &paramLen[0], isNull[0], error[0], noneInd);
    for (unsigned int i = 1; i < 1 + timestamps.size(); i++)
        setBind(bind[i], MYSQL_TYPE_TIMESTAMP, &timestamps[i - 1], sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[i], isNull[i], error[i], noneInd);
    setBind(bind[4], MYSQL_TYPE_STRING, fileDigest, digestSize, &paramLen[4], isNull[4], error[4], noneInd);
    setBind(bind[5], MYSQL_TYPE_STRING, fileType, digestSize, &paramLen[5], isNull[5], error[5], noneInd);
    for (unsigned int i = 6; i < 6 + versionInfo.size(); i++)
        setBind(bind[i], MYSQL_TYPE_STRING, versionInfo[i - 6].get(), versionSize, &paramLen[i], isNull[i], error[i], noneInd);

    if (mysql_stmt_bind_result(stmt, bind))
        return printErr("Bind MySQL statement results");

    if (mysql_stmt_store_result(stmt)) // Fetch the entire result set at once
        return printErr("Store MySQL statement results");

    return OK;
}

int DBConnection::executeInsert(char *fileName, time_t fileCreated, time_t fileChanged, char *fileDigest,
                                char *fileType, std::vector<std::shared_ptr<char[]>> &versionInfo)
{
    char ntsInd = STMT_INDICATOR_NTS;
    isNull[0] = false;

    paramLen[0] = std::strlen(fileName);
    for (int i = 1; i < 3; i++)
        paramLen[i] = sizeof(MYSQL_TYPE_TIMESTAMP);
    paramLen[3] = std::strlen(fileDigest);
    paramLen[4] = std::strlen(fileType);
    for (unsigned int i = 5; i < 5 + versionInfo.size(); i++)
        paramLen[i] = std::strlen(versionInfo[i].get());

    setBind(bind[0], MYSQL_TYPE_STRING, fileName, 0, &paramLen[0], isNull[0], error[0], ntsInd);
    setBind(bind[1], MYSQL_TYPE_TIMESTAMP, &fileCreated, 0, &paramLen[1], isNull[0], error[0], ntsInd);
    setBind(bind[2], MYSQL_TYPE_TIMESTAMP, &fileChanged, 0, &paramLen[2], isNull[0], error[0], ntsInd);
    setBind(bind[3], MYSQL_TYPE_STRING, fileDigest, 0, &paramLen[3], isNull[0], error[0], ntsInd);
    setBind(bind[4], MYSQL_TYPE_STRING, fileType, 0, &paramLen[4], isNull[0], error[0], ntsInd);
    for (unsigned int i = 5; i < 5 + versionInfo.size(); i++)
        setBind(bind[i], MYSQL_TYPE_STRING, versionInfo[i].get(), 0, &paramLen[i], isNull[0], error[0], ntsInd);

    return executeStmt();
}

/* Set and execute the prepared statement in order to get data from database */
int DBConnection::executeSelect(char *digest, char *name)
{
    char ignoreInd = STMT_INDICATOR_IGNORE;
    char ntsInd = STMT_INDICATOR_NTS;

    // Only two of indicators are necessary for bind when substituting for statement variables (?)
    isNull[0] = false;
    isNull[1] = true;
    paramLen[0] = std::strlen(digest);
    paramLen[1] = std::strlen(name);

    //Bind statement variables to their corresponding substitutions
    setBind(bind[0], MYSQL_TYPE_STRING, digest, 0, &paramLen[0], isNull[0], error[0], ntsInd);
    setBind(bind[1], MYSQL_TYPE_STRING, name, 0, &paramLen[1], isNull[0], error[0], ntsInd);
    setBind(bind[2], MYSQL_TYPE_STRING, digest, 0, &paramLen[0], isNull[0], error[0], ntsInd);
    for (int i = 3; i < MAX_ATTR_COUNT; i++) // Ignore the rest of bind structures this time
        setBind(bind[i], MYSQL_TYPE_STRING, NULL, 0, NULL, isNull[1], error[0], ignoreInd);

    return executeStmt();
}

int DBConnection::executeStmt()
{
    if (mysql_stmt_bind_param(stmt, bind))
        return printErr("Bind MySQL statement");
    if (mysql_stmt_execute(stmt))
        return printErr("Execute MySQL statement");

    return OK;
}

int DBConnection::fetchData()
{
    int rc = mysql_stmt_fetch(stmt);

    if (rc == FAIL)
        return printErr("Fetch MySQL statement results");
    else if (rc == MYSQL_DATA_TRUNCATED)
    {
        mysql_stmt_data_seek(stmt, 0);
        return MYSQL_DATA_TRUNCATED;
    }

    return rc;
}

int DBConnection::init(const char *query)
{
    mysql = mysql_init(NULL);
    if (!mysql_real_connect(mysql, hostName, userName, userPasswd, dbName, portNum, unixSocket, 0))
        return printErr("Open MySQL connection");

    stmt = mysql_stmt_init(mysql);
    if (mysql_stmt_prepare(stmt, query, std::strlen(query)))
        return printErr("Prepare MySQL statement");

    return OK;
}

/* Print error details */
int DBConnection::printErr(const char *errInfo)
{
    printFailed(errInfo);
    std::cerr << "Error(" << mysql_errno(mysql) << ") ["
              << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
    return FAIL;
}

/* Fill in parameters of bind structure used for definition of statement variables substitution
or definition of query result set storage and indicators */
void DBConnection::setBind(
    MYSQL_BIND &bind, enum enum_field_types field_type, void *param, size_t paramSize,
    size_t *paramLen, my_bool &isNull, my_bool &error, char &ind)
{
    bind.buffer_type = field_type;
    bind.buffer = param;
    bind.is_null = &isNull;
    bind.error = &error;
    bind.u.indicator = &ind;
    if (paramLen)
    {
        bind.length = (unsigned long *)paramLen;
        bind.buffer_length = !paramSize ? *paramLen : paramSize;
    }
}

void DBConnection::setSize(int nameSize, int digestSize, int versionSize)
{
    this->nameSize = nameSize;
    this->digestSize = digestSize;
    this->versionSize = versionSize;
}
