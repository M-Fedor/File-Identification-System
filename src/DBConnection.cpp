#include "DBConnection.h"

DBConnection::DBConnection() {}

DBConnection::DBConnection(
    const char *host, const char *user, const char *passwd,
    const char *db, unsigned int port, const char *unixSock)
    : dbName(db), hostName(host), unixSocket(unixSock), userName(user), userPasswd(passwd),
      digestSize(0), nameSize(0), versionSize(0), mysql(NULL), stmt(NULL), portNum(port)
{
    // Initialize indicators for query execution
    error = std::vector<my_bool>(8, 0);
    isNull = std::vector<my_bool>(8, 0);
    paramLen = std::vector<size_t>(8, 0);

    memset(&bind, 0, sizeof(MYSQL_BIND) * 8);
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

int DBConnection::bindResults(
    MYSQL_TIME fileCreated, MYSQL_TIME fileChanged, MYSQL_TIME fileRegistered,
    char *fileDigest, char *fileName, char *fileVersion, char *osCombination, char *swPackage)
{
    char noneInd = STMT_INDICATOR_NONE;

    setBind(bind[0], MYSQL_TYPE_STRING, fileName, nameSize, &paramLen[0], isNull[0], error[0], noneInd);
    setBind(bind[1], MYSQL_TYPE_TIMESTAMP, &fileCreated, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[1], isNull[1], error[1], noneInd);
    setBind(bind[2], MYSQL_TYPE_TIMESTAMP, &fileChanged, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[2], isNull[2], error[2], noneInd);
    setBind(bind[3], MYSQL_TYPE_TIMESTAMP, &fileRegistered, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[3], isNull[3], error[3], noneInd);
    setBind(bind[4], MYSQL_TYPE_STRING, fileDigest, digestSize, &paramLen[4], isNull[4], error[4], noneInd);
    setBind(bind[5], MYSQL_TYPE_STRING, fileVersion, versionSize, &paramLen[5], isNull[5], error[5], noneInd);
    setBind(bind[6], MYSQL_TYPE_STRING, swPackage, versionSize, &paramLen[6], isNull[6], error[6], noneInd);
    setBind(bind[7], MYSQL_TYPE_STRING, osCombination, versionSize, &paramLen[7], isNull[7], error[7], noneInd);

    if (mysql_stmt_bind_result(stmt, bind))
        return printErr("Bind MySQL statement results");

    if (mysql_stmt_store_result(stmt)) // Fetch the entire result set at once
        return printErr("Store MySQL statement results");

    return OK;
}

/* Set and execute the prepared statement in order to get data from database */
int DBConnection::executeSelect(std::string &digest, std::string &name)
{
    char ignoreInd = STMT_INDICATOR_IGNORE;
    char ntsInd = STMT_INDICATOR_NTS;

    // Only two of indicators are necessary for bind when substituting for statement variables (?)
    isNull[0] = false;
    isNull[1] = true;
    paramLen[0] = digest.size();
    paramLen[1] = name.size();

    std::unique_ptr<char[]> digestStr(new char[digest.size()]);
    std::unique_ptr<char[]> nameStr(new char[name.size()]);
    std::strncpy(digestStr.get(), digest.data(), digest.size());
    std::strncpy(nameStr.get(), name.data(), name.size());

    //Bind statement variables to their corresponding substitutions
    setBind(bind[0], MYSQL_TYPE_STRING, digestStr.get(), 0, &paramLen[0], isNull[0], error[0], ntsInd);
    setBind(bind[1], MYSQL_TYPE_STRING, nameStr.get(), 0, &paramLen[1], isNull[0], error[0], ntsInd);
    setBind(bind[2], MYSQL_TYPE_STRING, digestStr.get(), 0, &paramLen[0], isNull[0], error[0], ntsInd);
    for (int i = 3; i < 8; i++) // Ignore the rest of bind structures this time
        setBind(bind[i], MYSQL_TYPE_STRING, NULL, 0, NULL, isNull[1], error[0], ignoreInd);

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
        return 2;
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
