#include "OutputValidateDB.h"

/* Constructor, set up the credentials for communication with DBMS */
OutputValidateDB::OutputValidateDB(DBConnection &conn, std::shared_ptr<OutputOffline> out)
    : bufferSizeFactor(1), fOutput(out)
{
    connection = std::move(conn);

    fileDigest = std::unique_ptr<char[]>(new char[DIGEST_SIZE]);
    fileName = std::unique_ptr<char[]>(new char[NAME_SIZE]);
    fileVersion = std::unique_ptr<char[]>(new char[VERSION_SIZE]);
    osCombination = std::unique_ptr<char[]>(new char[VERSION_SIZE]);
    swPackage = std::unique_ptr<char[]>(new char[VERSION_SIZE]);
}

/* Destructor */
OutputValidateDB::~OutputValidateDB() {}

/* Format data obtained from database into convenient form */
int OutputValidateDB::formatData(std::string &digest, std::string &name, std::string &data)
{
    bool foundResult = false;
    std::stringstream outputStr;

    outputStr << name << "\n"
              << digest << "\n";

    int rc = connection.fetchData();
    while (!rc)
    {
        if (!foundResult)
            foundResult = true;
        if (!strcmp(fileDigest.get(), digest.data()))
        {
            if (!strcmp(fileName.get(), name.data()))
                outputStr << "\tVALID_FILE\n";
            else
                outputStr << "\tDIFFERENT_FILE_NAME_OR_FILE_LOCATION\n";
        }
        else
            outputStr << "\tSUSPICIOS_FILE\n";
        outputStr << "\t\t" << fileName.get() << ", " << fileCreated.day << "." << fileCreated.month << "."
                  << fileCreated.year << " " << fileCreated.hour << ":" << fileCreated.minute << ":"
                  << fileCreated.second << ", " << fileChanged.day << "." << fileChanged.month << "."
                  << fileChanged.year << " " << fileChanged.hour << ":" << fileChanged.minute << ":"
                  << fileChanged.second << ", " << fileRegistered.day << "." << fileRegistered.month << "."
                  << fileRegistered.year << " " << fileRegistered.hour << ":" << fileRegistered.minute << ":"
                  << fileRegistered.second << fileDigest.get() << ", " << fileVersion.get() << ", "
                  << swPackage.get() << ", " << osCombination.get() << "\n";
        rc = connection.fetchData();
    }

    if (rc == 1)
        return 1;
    else if (rc == 2)
    {
        printWarning("Data truncated, resizing buffers");
        resizeBuffers();
        return 2;
    }

    if (!foundResult)
        outputStr << "FILE_NOT_FOUND\n";
    data = outputStr.str();

    return 0;
}

/* Initialize communication with DBMS, open output file and prepare statement to be executed, report any failures */
int OutputValidateDB::init()
{
    if (connection.init("(SELECT file_name, file_created, file_changed, file_registered, file_digest,"
                        " file_version, sw_package, os_combination"
                        " FROM fileinfo WHERE file_digest = ?) UNION"
                        " (SELECT file_name, file_created, file_changed, file_registered, file_digest,"
                        " file_version, sw_package, os_combination"
                        " FROM fileinfo WHERE file_name = ? AND file_digest != ?)"))
        return 1;
    connection.setSize(NAME_SIZE, DIGEST_SIZE, VERSION_SIZE);

    if (fOutput->init())
        return 1;

    return 0;
}

/* Get data from database and output them in output file */
int OutputValidateDB::outputData(std::string &digest, std::string &name)
{
    if (connection.executeSelect(digest, name))
        return 1;
    if (connection.bindResults(
            fileCreated, fileChanged, fileRegistered, fileDigest.get(),
            fileName.get(), fileVersion.get(), osCombination.get(), swPackage.get()))
        return 1;

    int rc = 0;
    std::string data;
    do
    {
        rc = formatData(digest, name, data);
        if (rc == 1)
            return rc;
    } while (rc == 2);

    fOutput->outputData(data);
    return 0;
}

/* Resize buffers for data from database, allows us to adapt to various conditions
and versions of our databases */
void OutputValidateDB::resizeBuffers()
{
    bufferSizeFactor *= 2;

    fileName.reset(new char[NAME_SIZE * bufferSizeFactor]);
    fileVersion.reset(new char[VERSION_SIZE * bufferSizeFactor]);
    osCombination.reset(new char[VERSION_SIZE * bufferSizeFactor]);
    swPackage.reset(new char[VERSION_SIZE * bufferSizeFactor]);

    connection.setSize(
        NAME_SIZE * bufferSizeFactor, DIGEST_SIZE, VERSION_SIZE * bufferSizeFactor);
}
