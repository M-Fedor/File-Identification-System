#include "OutputValidateDB.h"

/* Constructor, set up the credentials for communication with DBMS */
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

void OutputValidateDB::evaluateData(std::string &digest, std::string &name, std::stringstream &str)
{
    if (!strcmp(fileDigest.get(), digest.data()))
    {
        if (!strcmp(fileName.get(), name.data()))
            str << "\tVALID_FILE\n";
        else
            str << "\tDIFFERENT_FILE_NAME_OR_FILE_LOCATION\n";
    }
    else
        str << "\tSUSPICIOS_FILE\n";
}

/* Format data obtained from database into convenient form */
int OutputValidateDB::formatData(std::string &digest, std::string &name, std::string &data)
{
    bool resultNotFound = true;
    std::stringstream outputStr;

    outputStr << name << "\n"
              << digest << "\n";

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
        outputStr << "FILE_UNKNOWN\n";
    data = std::move(outputStr.str());

    return OK;
}

int OutputValidateDB::getData(std::string &digest, std::string &name)
{
    std::unique_ptr<char[]> digestStr(new char[digest.size() + 1]);
    std::unique_ptr<char[]> nameStr(new char[name.size() + 1]);
    std::strncpy(digestStr.get(), digest.data(), digest.size() + 1);
    std::strncpy(nameStr.get(), name.data(), name.size() + 1);

    return connection.executeSelect(digestStr.get(), nameStr.get())
               ? FAIL
               : connection.bindResults(
                     fileName.get(), timestamps, fileDigest.get(), fileType.get(), versionInfo);
}

/* Initialize communication with DBMS, open output file and prepare statement to be executed, report any failures */
int OutputValidateDB::init()
{
    if (connection.init("(SELECT file_name, file_created, file_changed, file_registered, file_digest, file_type"
                        " company_name, product_name, product_version, file_version, file_description, os_combination"
                        " FROM fileinfo WHERE file_digest = ?) UNION"
                        " (SELECT file_name, file_created, file_changed, file_registered, file_digest, file_type"
                        " company_name, product_name, product_version, file_version, file_description, os_combination"
                        " FROM fileinfo WHERE file_name = ? AND file_digest != ?)"))
        return FAIL;
    connection.setSize(NAME_SIZE, DIGEST_SIZE, VERSION_SIZE);

    if (fOutput->init())
        return FAIL;

    return OK;
}

void OutputValidateDB::makePartialOut(
    std::string &digest, std::string &name, std::stringstream &str)
{
    evaluateData(digest, name, str);
    str << "\t\t" << fileName.get() << ", " << fileDigest.get()
        << ", " << fileType.get() << ", ";

    for (auto &time : timestamps)
        str << time.day << "." << time.month << "." << time.year << " "
            << time.hour << ":" << time.minute << ":" << time.second << ", ";

    for (auto &info : versionInfo)
        str << ", " << info.get();
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
