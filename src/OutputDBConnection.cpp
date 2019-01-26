#include "OutputDBConnection.h"

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

    fileDigest = new char[DIGEST_SIZE];
    fileName = new char[NAME_SIZE];
    fileVersion = new char[VERSION_SIZE];

    error = std::vector<my_bool>(5, 0);
    isNull = std::vector<my_bool>(5, 0);
    paramLen = std::vector<size_t>(5, 0);

    memset(&bind, 0, sizeof(MYSQL_BIND) * 5);
}
OutputDBConnection::~OutputDBConnection()
{
    if (mysql_stmt_close(getDigestFileName))
    {
        printf("\033[31mFAILED\033[0m to close MySQL statement\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_errno(mysql),
               mysql_sqlstate(mysql), mysql_error(mysql));
    }

    mysql_close(mysql);

    delete[] fileDigest;
    delete[] fileName;
    delete[] fileVersion;
    free(getDigestFileNameStr);

    if (fclose(fWrite))
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to close \033[1mValidation_results.txt\033[0m\n");
        perror(strerror(temp_errno));
    }
}

int OutputDBConnection::init()
{
    mysql = mysql_init(NULL);
    if (!mysql_real_connect(mysql, hostName, userName, userPasswd, dbName, portNum, unixSocket, 0))
    {
        printf("\033[31mFAILED\033[0m to open MySQL connection\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_errno(mysql),
               mysql_sqlstate(mysql), mysql_error(mysql));
        return 1;
    }

    getDigestFileName = mysql_stmt_init(mysql);
    getDigestFileNameStr = strdup("(SELECT file_name, file_created, file_changed, file_digest, file_version"
                                  " FROM fileinfo WHERE file_digest = ?) UNION"
                                  " (SELECT file_name, file_created, file_changed, file_digest, file_version"
                                  " FROM fileinfo WHERE file_name = ? AND file_digest != ?)");

    if (mysql_stmt_prepare(getDigestFileName, getDigestFileNameStr, strlen(getDigestFileNameStr)))
    {
        printf("\033[31mFAILED\033[0m to prepare MySQL statement\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }

    fWrite = fopen("Validation_results.txt", "w");
    if (!fWrite)
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to open \033[1mValidation_results.txt\033[0m\n");
        perror(strerror(temp_errno));
        return 1;
    }

    return 0;
}

void OutputDBConnection::resizeBuffers()
{
    bufferSizeCoefficient *= 2;

    delete[] fileName;
    delete[] fileVersion;

    fileName = new char[NAME_SIZE * bufferSizeCoefficient];
    fileVersion = new char[VERSION_SIZE * bufferSizeCoefficient];
}

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

int OutputDBConnection::outputData(std::string digest, std::string name)
{
    bool foundResult = false;
    char ignoreInd = STMT_INDICATOR_IGNORE;
    char noneInd = STMT_INDICATOR_NONE;
    char ntsInd = STMT_INDICATOR_NTS;

    isNull[0] = false;
    isNull[1] = true;
    paramLen[0] = strlen(digest.data());
    paramLen[1] = strlen(name.data());

    setBind(bind[0], MYSQL_TYPE_STRING, strdup(digest.data()), 0, &paramLen[0], isNull[0], error[0], ntsInd);
    setBind(bind[1], MYSQL_TYPE_STRING, strdup(name.data()), 0, &paramLen[1], isNull[0], error[0], ntsInd);
    setBind(bind[2], MYSQL_TYPE_STRING, strdup(digest.data()), 0, &paramLen[0], isNull[0], error[0], ntsInd);

    for (int i = 3; i < 5; i++)
        setBind(bind[i], MYSQL_TYPE_STRING, NULL, 0, NULL, isNull[1], error[0], ignoreInd);

    if (mysql_stmt_bind_param(getDigestFileName, bind))
    {
        printf("\033[31mFAILED\033[0m to bind MySQL statement\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }

    if (mysql_stmt_execute(getDigestFileName))
    {
        printf("\033[31mFAILED\033[0m to execute MySQL statement\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }

    for (int i = 0; i < 3; i++)
        free(bind[i].buffer);

    setBind(bind[0], MYSQL_TYPE_STRING, fileName, NAME_SIZE, &paramLen[0], isNull[0], error[0], noneInd);
    setBind(bind[1], MYSQL_TYPE_TIMESTAMP, &fileCreated, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[1], isNull[1], error[1], noneInd);
    setBind(bind[2], MYSQL_TYPE_TIMESTAMP, &fileChanged, sizeof(MYSQL_TYPE_TIMESTAMP), &paramLen[2], isNull[2], error[2], noneInd);
    setBind(bind[3], MYSQL_TYPE_STRING, fileDigest, DIGEST_SIZE, &paramLen[3], isNull[3], error[3], noneInd);
    setBind(bind[4], MYSQL_TYPE_STRING, fileVersion, VERSION_SIZE, &paramLen[4], isNull[4], error[4], noneInd);

    if (mysql_stmt_bind_result(getDigestFileName, bind))
    {
        printf("\033[31mFAILED\033[0m to bind MySQL statement results\n");
        printf("Error(%d) [%s] \"%s\"", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }

    if (mysql_stmt_store_result(getDigestFileName))
    {
        printf("\033[31mFAILED\033[0m to store MySQL statement results\n");
        printf("Error(%d) [%s] \"%s\"", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }

    fprintf(fWrite, "%s\n%s\n", name.data(), digest.data());

    int rc = mysql_stmt_fetch(getDigestFileName);
    while (!rc)
    {
        if (!foundResult)
            foundResult = true;

        if (!strcmp(fileDigest, digest.data()))
        {
            if (!strcmp(fileName, name.data()))
                fprintf(fWrite, "\tVALID_FILE\n");
            else
                fprintf(fWrite, "\tDIFFERENT_FILE_NAME_OR_FILE_LOCATION\n");
        }
        else
            fprintf(fWrite, "\tSUSPICIOS_FILE\n");

        fprintf(fWrite, "\t\t%s, %02d.%02d.%04d %02d:%02d:%02d, %02d.%02d.%04d %02d:%02d:%02d, %s, %s\n",
                fileName, fileCreated.day, fileCreated.month, fileCreated.year, fileCreated.hour, fileCreated.minute, fileCreated.second,
                fileChanged.day, fileChanged.month, fileChanged.year, fileChanged.hour, fileChanged.minute, fileChanged.second,
                fileDigest, fileVersion);

        rc = mysql_stmt_fetch(getDigestFileName);
    }

    if (rc == 1)
    {
        printf("\033[31mFAILED\033[0m to fetch MySQL statement results\n");
        printf("Error(%d) [%s] \"%s\"\n", mysql_stmt_errno(getDigestFileName),
               mysql_stmt_sqlstate(getDigestFileName), mysql_stmt_error(getDigestFileName));
        return 1;
    }
    else if (rc == MYSQL_DATA_TRUNCATED)
    {
        printf("\033[33mWARNING:\033[0m data truncated, resizing buffers...\n");
        resizeBuffers();
        return 2;
    }

    if (!foundResult)
        fprintf(fWrite, "FILE_NOT_FOUND\n");
    fprintf(fWrite, "\n");

    return 0;
}