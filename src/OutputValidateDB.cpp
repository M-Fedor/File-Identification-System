#include "OutputValidateDB.h"

/* Constructor; set up buffers for communication with DBMS */
OutputValidateDB::OutputValidateDB(DBConnection &conn, std::shared_ptr<OutputOffline> out)
    : bufferSizeFactor(1), fOutput(out)
{
    connection = std::move(conn);

    fileDigest.reset(new char[DIGEST_SIZE]);
    fileName.reset(new char[NAME_SIZE]);
    fileType.reset(new char[DIGEST_SIZE]);

    timestamps = std::vector<MYSQL_TIME>(TIMESTAMPS_INFO_ATTR_COUNT);
    for (int i = 0; i < VERSION_INFO_ATTR_COUNT; i++)
        versionInfo.emplace_back(new char[VERSION_SIZE]);
}

/* Destructor */
OutputValidateDB::~OutputValidateDB() {}

/* Determine nature of examined file */
void OutputValidateDB::evaluateData(std::string &digest, std::string &name, std::stringstream &str)
{
    const char *status;

    status = (!std::strcmp(fileName.get(), name.data())) ? "valid," : "warning,";
    status = (std::strcmp(fileDigest.get(), digest.data())) ? "suspicious," : status;

    str << status;
}

/* Format data obtained from database into convenient form */
int OutputValidateDB::formatData(std::string &digest, std::string &name, std::string &data)
{
    if (connection.bindResults(
            fileName.get(), fileDigest.get(), timestamps, fileType.get(), versionInfo))
        return FAIL;

    bool resultNotFound = true;
    std::stringstream outputStr;

    int rc = connection.fetchData();
    resultNotFound = !rc ? false : true;
    while (!rc)
    {
        makePartialOut(digest, name, outputStr);
        rc = connection.fetchData();
    }

    if (rc == 1)
        return FAIL;
    else if (rc == MYSQL_DATA_TRUNCATED)
    {
        printWarning("Data truncated, resizing buffers");
        resizeBuffers();
        return MYSQL_DATA_TRUNCATED;
    }

    if (resultNotFound)
    {
        outputStr << "unknown," << name << "," << digest;
        for(int i = 0; i < MAX_ATTR_COUNT; i++)
            outputStr << ",";
        outputStr << "\n";
    }

    data = std::move(outputStr.str());

    return OK;
}

/* Prepare buffers for prepared statement variable substitution and
obtain file-related data from database */
int OutputValidateDB::getData(std::string &digest, std::string &name)
{
    std::unique_ptr<char[]> digestStr(new char[digest.size() + 1]);
    std::unique_ptr<char[]> nameStr(new char[name.size() + 1]);
    std::strncpy(digestStr.get(), digest.data(), digest.size() + 1);
    std::strncpy(nameStr.get(), name.data(), name.size() + 1);

    return connection.executeSelect(digestStr.get(), nameStr.get());
}

/* Initialize communication with DBMS, open output file and initialize database connection, report any failures */
int OutputValidateDB::init()
{
    if (connection.init("(SELECT * FROM recognize_file WHERE file_digest = ?) UNION"
                        "(SELECT * FROM recognize_file WHERE absolute_path = ? AND file_digest != ?);"))
        return FAIL;
    connection.setSize(NAME_SIZE, DIGEST_SIZE, VERSION_SIZE);

    if (fOutput->init())
        return FAIL;

    return OK;
}

/* Produce an output result-string for examined file */
void OutputValidateDB::makePartialOut(
    std::string &digest, std::string &name, std::stringstream &str)
{
    evaluateData(digest, name, str);
    str << name << "," << digest << "," << fileName.get() << "," << fileDigest.get();

    for (auto &time : timestamps)
        str << "," << time.day << "." << time.month << "." << time.year << " "
            << time.hour << ":" << time.minute << ":" << time.second;

    str << "," << fileType.get();

    for (auto &info : versionInfo)
        str << "," << info.get();
    str << "\n";
}

/* Get data from database and output them in output file */
int OutputValidateDB::outputData(std::string &digest, std::string &name)
{
    if (getData(digest, name))
        return FAIL;

    int rc = OK;
    std::string data;
    do
    {
        rc = formatData(digest, name, data);
        if (rc == FAIL)
            return FAIL;
    } while (rc == MYSQL_DATA_TRUNCATED);

    fOutput->outputData(data);
    return OK;
}

/* Resize buffers for data from database, allows us to adapt to various conditions
and versions of our databases */
void OutputValidateDB::resizeBuffers()
{
    bufferSizeFactor *= 2;

    fileName.reset(new char[NAME_SIZE * bufferSizeFactor]);
    for (auto &info : versionInfo)
        info.reset((new char[VERSION_SIZE * bufferSizeFactor]));

    connection.setSize(
        NAME_SIZE * bufferSizeFactor, DIGEST_SIZE, VERSION_SIZE * bufferSizeFactor);
}
