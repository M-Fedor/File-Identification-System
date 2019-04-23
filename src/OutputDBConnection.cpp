#include "OutputDBConnection.h"

/* Constructor, set up the credentials for communication with DBMS */
OutputDBConnection::OutputDBConnection(
    char *hostName, char *userName, char *userPasswd, char *dbName, unsigned int portNum, char *unixSocket)
    : bufferSizeCoefficient(1)
{
    this->hostName = hostName;
    this->userName = userName;
    this->userPasswd = userPasswd;
    this->dbName = dbName;
    this->portNum = portNum;
    this->unixSocket = unixSocket;

    // Allocate buffers for data from database
    fileDigest = new char[DIGEST_SIZE];
    fileName = new char[NAME_SIZE];
    fileVersion = new char[VERSION_SIZE];

    // Initialize indicators for query execution
    error = std::vector<my_bool>(5, 0);
    isNull = std::vector<my_bool>(5, 0);
    paramLen = std::vector<size_t>(5, 0);

    memset(&bind, 0, sizeof(MYSQL_BIND) * 5);
}

/* Destructor */
OutputDBConnection::~OutputDBConnection()
{
    if (mysql_stmt_close(getDigestFileName))
    {
        std::cerr << "\033[31mFAILED\033[0m to close MySQL statement\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
    }

    mysql_close(mysql);

    delete[] fileDigest;
    delete[] fileName;
    delete[] fileVersion;
    free(getDigestFileNameStr);

    fOutput.close();
    if (fOutput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close \033[1mValidation_results.txt\033[0m\n";
}

/* Initialize communication with DBMS, open output file and prepare statement to be executed, report any failures */
int OutputDBConnection::init()
{
    mysql = mysql_init(NULL);
    if (!mysql_real_connect(mysql, hostName, userName, userPasswd, dbName, portNum, unixSocket, 0))
    {
        std::cerr << "\033[31mFAILED\033[0m to open MySQL connection\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    getDigestFileName = mysql_stmt_init(mysql);
    getDigestFileNameStr = strdup("(SELECT file_name, file_created, file_changed, file_digest, file_version"
                                  " FROM fileinfo WHERE file_digest = ?) UNION"
                                  " (SELECT file_name, file_created, file_changed, file_digest, file_version"
                                  " FROM fileinfo WHERE file_name = ? AND file_digest != ?)");

    if (mysql_stmt_prepare(getDigestFileName, getDigestFileNameStr, strlen(getDigestFileNameStr)))
    {
        std::cerr << "\033[31mFAILED\033[0m to prepare MySQL statement\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    fOutput.open("Validation_results.txt", std::ios::trunc);
    if (fOutput.fail())
    {
        std::cerr << "\033[31mFAILED\033[0m to open \033[1mValidation_results.txt\033[0m\n";
        return 1;
    }

    return 0;
}

/* Resize buffers for data from database, allows us to adapt to various conditions
and versions of our databases */
void OutputDBConnection::resizeBuffers()
{
    bufferSizeCoefficient *= 2;

    delete[] fileName;
    delete[] fileVersion;

    fileName = new char[NAME_SIZE * bufferSizeCoefficient];
    fileVersion = new char[VERSION_SIZE * bufferSizeCoefficient];
}

/* Fill in parameters of bind structure used for definition of statement variables substitution
or definition of query result set storage and indicators */
void OutputDBConnection::setBind(
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
        bind.length = paramLen;

        if (!paramSize)
            bind.buffer_length = *paramLen;
        else
            bind.buffer_length = paramSize;
    }
}

/* Get data from database and output them in output file */
int OutputDBConnection::outputData(std::string digest, std::string name)
{
    bool foundResult = false;
    char ignoreInd = STMT_INDICATOR_IGNORE;
    char noneInd = STMT_INDICATOR_NONE;
    char ntsInd = STMT_INDICATOR_NTS;

    // Only two of indicators are necessary for bind when substituting for statement variables (?)
    isNull[0] = false;
    isNull[1] = true;
    paramLen[0] = strlen(digest.data());
    paramLen[1] = strlen(name.data());

    //Bind statement variables to their corresponding substitutions
    setBind(bind[0], MYSQL_TYPE_STRING, strdup(digest.data()), 0, &paramLen[0], isNull[0], error[0], ntsInd);
    setBind(bind[1], MYSQL_TYPE_STRING, strdup(name.data()), 0, &paramLen[1], isNull[0], error[0], ntsInd);
    setBind(bind[2], MYSQL_TYPE_STRING, strdup(digest.data()), 0, &paramLen[0], isNull[0], error[0], ntsInd);

    // Ignore the rest of bind structures this time
    for (int i = 3; i < 5; i++)
        setBind(bind[i], MYSQL_TYPE_STRING, NULL, 0, NULL, isNull[1], error[0], ignoreInd);

    if (mysql_stmt_bind_param(getDigestFileName, bind))
    {
        std::cerr << "\033[31mFAILED\033[0m to bind MySQL statement\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    if (mysql_stmt_execute(getDigestFileName))
    {
        std::cerr << "\033[31mFAILED\033[0m to execute MySQL statement\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    for (int i = 0; i < 3; i++)
        free(bind[i].buffer);

    // Bind corresponding storage to atributes of result set
    setBind(bind[0], MYSQL_TYPE_STRING, fileName, NAME_SIZE, &paramLen[0], isNull[0], error[0], noneInd);
    setBind(bind[1], MYSQL_TYPE_TIMESTAMP, &fileCreated, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[1], isNull[1], error[1], noneInd);
    setBind(bind[2], MYSQL_TYPE_TIMESTAMP, &fileChanged, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[2], isNull[2], error[2], noneInd);
    setBind(bind[3], MYSQL_TYPE_STRING, fileDigest, DIGEST_SIZE, &paramLen[3], isNull[3], error[3], noneInd);
    setBind(bind[4], MYSQL_TYPE_STRING, fileVersion, VERSION_SIZE, &paramLen[4], isNull[4], error[4], noneInd);

    if (mysql_stmt_bind_result(getDigestFileName, bind))
    {
        std::cerr << "\033[31mFAILED\033[0m to bind MySQL statement results\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    // Fetch the entire result set at once
    if (mysql_stmt_store_result(getDigestFileName))
    {
        std::cerr << "\033[31mFAILED\033[0m to store MySQL statement results\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }

    // Create formatted output
    fOutput << name.data() << "\n"
            << digest.data() << "\n";

    int rc = mysql_stmt_fetch(getDigestFileName);
    while (!rc)
    {
        if (!foundResult)
            foundResult = true;

        if (!strcmp(fileDigest, digest.data()))
        {
            if (!strcmp(fileName, name.data()))
                fOutput << "\tVALID_FILE\n";
            else
                fOutput << "\tDIFFERENT_FILE_NAME_OR_FILE_LOCATION\n";
        }
        else
            fOutput << "\tSUSPICIOS_FILE\n";

        fOutput << "\t\t" << fileName << ", " << fileCreated.day << "." << fileCreated.month << "."
                << fileCreated.year << " " << fileCreated.hour << ":" << fileCreated.minute << ":"
                << fileCreated.second << ", " << fileChanged.day << "." << fileChanged.month << "."
                << fileChanged.year << " " << fileChanged.hour << ":" << fileChanged.minute << ":"
                << fileChanged.second << ", " << fileDigest << ", " << fileVersion << "\n";

        rc = mysql_stmt_fetch(getDigestFileName);
    }

    if (rc == 1)
    {
        std::cerr << "\033[31mFAILED\033[0m to fetch MySQL statement results\n";
        std::cerr << "Error(" << mysql_errno(mysql) << ") ["
                  << mysql_sqlstate(mysql) << "] \"" << mysql_error(mysql) << "\"\n";
        return 1;
    }
    else if (rc == MYSQL_DATA_TRUNCATED)
    {
        std::cout << "\033[33mWARNING:\033[0m data truncated, resizing buffers...\n";
        resizeBuffers();
        return 2;
    }

    if (!foundResult)
        fOutput << "FILE_NOT_FOUND\n";
    fOutput << "\n";

    return 0;
}