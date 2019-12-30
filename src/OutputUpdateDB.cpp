#include "OutputUpdateDB.h"

#if defined(_WIN32)

/* Constructor; set up buffers and data structures for information obtainment */
OutputUpdateDB::OutputUpdateDB(DBConnection &conn)
    : verStr(NULL)
{
    connection = std::move(conn);

    defaultStr.reset(new char[strlen("Unknown") + 1]);
    std::strncpy(defaultStr.get(), "Unknown", strlen("Unknown") + 1);

    versionAttributes = std::vector<LPCSTR>{"CompanyName", "ProductName", "ProductVersion",
                                            "FileVersion", "FileDescription"};
    versionInfo = std::vector<char *>(VERSION_INFO_ATTR_COUNT, NULL);

    translationList[0].language = translationList[1].language = std::string("0409");
    translationList[2].language = translationList[3].language = std::string("0000");
    translationList[0].codePage = translationList[2].codePage = std::string("04B0");
    translationList[1].codePage = translationList[3].codePage = std::string("04E4");
}

/* Destructor */
OutputUpdateDB::~OutputUpdateDB() {}

/* Get as much relevant information about file as possible */
int OutputUpdateDB::getFileInfo(std::string &name)
{
    DWORD dwHandle;

    if (_stat64(name.data(), &buffer))
        return printErr(errno, "Get _stat64 file info");

    verSize = GetFileVersionInfoSizeA(name.data(), &dwHandle);
    if (!verSize)
        return FAIL;
    verInfo.reset(new char[verSize]);
    if (!GetFileVersionInfoA(name.data(), dwHandle, verSize, (LPVOID)verInfo.get()))
        return FAIL;

    getType();
    if (getVariableVersion())
        getFixedVersion();

    return OK;
}

/* Get file version information from fixed non-optional binary part of VersionInfo memory block */
int OutputUpdateDB::getFixedVersion()
{
    std::stringstream str;
    if (!versionInfo[2])
    {
        str << HIWORD(fixedVerInfo->dwProductVersionMS) << "." << LOWORD(fixedVerInfo->dwProductVersionMS) << "."
            << HIWORD(fixedVerInfo->dwProductVersionLS) << "." << LOWORD(fixedVerInfo->dwProductVersionLS);
        productVerHelperStr = std::move(str.str());
    }

    str.str("");
    if (!versionInfo[3])
    {
        str << HIWORD(fixedVerInfo->dwFileVersionMS) << "." << LOWORD(fixedVerInfo->dwFileVersionMS) << "."
            << HIWORD(fixedVerInfo->dwFileVersionLS) << "." << LOWORD(fixedVerInfo->dwFileVersionLS);
        fileVerHelperStr = std::move(str.str());
    }

    return OK;
}

/* Get information about operating system currently in use */
int OutputUpdateDB::getOSVersion()
{
    const char *osVersion;
    if (IsWindows10OrGreater())
    {
        if (IsWindowsServer())
            osVersion = "Windows Server 2016 or later";
        else
            osVersion = "Windows 10";
    }

    else if (IsWindows8Point1OrGreater())
    {
        if (IsWindowsServer())
            osVersion = "Windows Server 2012 R2";
        else
            osVersion = "Windows 8.1";
    }

    else if (IsWindows8OrGreater())
    {
        if (IsWindowsServer())
            osVersion = "Windows Server 2012";
        else
            osVersion = "Windows 8";
    }

    else if (IsWindows7OrGreater())
    {
        if (IsWindowsServer())
            osVersion = "Windows Server 2008 R2";
        else
            osVersion = "Windows 7";
    }
    else
        return printFailed("Get OS version");

    osVerStr.reset(new char[strlen(osVersion) + 1]);
    std::strncpy(osVerStr.get(), osVersion, strlen(osVersion) + 1);
    versionInfo[5] = osVerStr.get();

    return OK;
}

/* Get file type from fixed non-optional binary part of VersionInfo memory block */
int OutputUpdateDB::getType()
{
    if (!VerQueryValue(
            (LPVOID)verInfo.get(), TEXT("\\"), (LPVOID *)&fixedVerInfo, (PUINT)&verSize))
    {
        fileType = "Unknown";
        return FAIL;
    }

    switch (fixedVerInfo->dwFileType)
    {
    case VFT_APP:
        fileType = "Application file";
        break;
    case VFT_DLL:
        fileType = "Dynamic-link library";
        break;
    case VFT_DRV:
        fileType = "Driver";
        break;
    case VFT_FONT:
        fileType = "Font file";
        break;
    case VFT_STATIC_LIB:
        fileType = "Static library";
        break;
    case VFT_UNKNOWN:
        fileType = "Unknown";
    }

    return OK;
}

/* Get file version information from variable optional text part of VersionInfo memory block */
int OutputUpdateDB::getVariableVersion()
{
    bool foundResult = false;
    std::stringstream str;
    for (auto &trnsltion : translationList)
    {
        str << "\\StringFileInfo\\" << trnsltion.language << trnsltion.codePage << "\\";

        for (int j = 0; j < VERSION_INFO_ATTR_COUNT - 1; j++)
        {
            if (VerQueryValue((LPVOID)verInfo.get(), str.str().append(versionAttributes[j]).data(),
                              (LPVOID *)&verStr, (PUINT)&verSize))
                versionInfo[j] = verStr;
            foundResult = versionInfo[j] ? true : false;
        }
        if (foundResult)
            break;
    }

    return !foundResult;
}

/* Initialize connection to database and determine current operating system,
report any failures */
int OutputUpdateDB::init()
{
    if (connection.init("INSERT INTO fileinfo"
                        " (file_name, file_created, file_changed, file_registered,"
                        " file_digest, file_type, company_name, product_name,"
                        " product_version, file_version, file_description, os_combination)"
                        " VALUES (?,  FROM_UNIXTIME(?) , FROM_UNIXTIME(?) , NOW(),"
                        " ?, ?, ?, ?, ?, ?, ?, ?)"))
        return FAIL;
    if (getOSVersion())
        return FAIL;

    return OK;
}

/* Prepare buffers for prepared statement variable substitution and 
insert extracted file information into database */
int OutputUpdateDB::insertData(std::string &digest, std::string &name)
{
    std::unique_ptr<char[]> digestStr(new char[digest.size() + 1]);
    std::unique_ptr<char[]> nameStr(new char[name.size() + 1]);
    std::unique_ptr<char[]> typeStr(new char[fileType.size() + 1]);

    std::strncpy(digestStr.get(), digest.data(), digest.size() + 1);
    std::strncpy(nameStr.get(), name.data(), name.size() + 1);
    std::strncpy(typeStr.get(), fileType.data(), fileType.size() + 1);

    std::unique_ptr<char[]> productVerStr, fileVerStr;
    if (!productVerHelperStr.empty())
    {
        productVerStr.reset(new char[productVerHelperStr.size() + 1]);
        std::strncpy(productVerStr.get(), productVerHelperStr.data(), productVerHelperStr.size() + 1);
        versionInfo[2] = productVerStr.get();
    }
    if (!fileVerHelperStr.empty())
    {
        fileVerStr.reset(new char[fileVerHelperStr.size() + 1]);
        std::strncpy(fileVerStr.get(), fileVerHelperStr.data(), fileVerHelperStr.size() + 1);
        versionInfo[3] = fileVerStr.get();
    }

    for (auto &info : versionInfo)
    {
        if (!info)
            info = defaultStr.get();
    }

    int rc = connection.executeInsert(
        nameStr.get(), buffer.st_ctime, buffer.st_mtime, digestStr.get(), typeStr.get(), versionInfo);

    productVerHelperStr.clear();
    fileVerHelperStr.clear();
    for (int i = 0; i < VERSION_INFO_ATTR_COUNT - 1; i++)
        versionInfo[i] = NULL;

    return rc;
}

/* Update database with extracted metadata related to files of interest */
int OutputUpdateDB::outputData(std::string &digest, std::string &name)
{
    getFileInfo(name);
    return insertData(digest, name);
}

/* Print error message in common format along with specific error description */
int OutputUpdateDB::printErr(int errNum, const char *errInfo)
{
    printFailed(errInfo);
    std::cerr << " - " << strerror(errNum) << "\n";
    return FAIL;
}

#endif
