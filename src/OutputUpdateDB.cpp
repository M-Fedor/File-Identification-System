#if defined(_WIN32)
#include "OutputUpdateDB.h"

OutputUpdateDB::OutputUpdateDB(DBConnection &conn)
    : verStr(NULL)
{
    connection = std::move(conn);
    versionAttributes = std::vector<LPCSTR>{"CompanyName", "ProductName", "ProductVersion",
                                            "FileVersion", "FileDescription"};
    versionInfo = std::vector<std::shared_ptr<char[]>>(VERSION_INFO_ATTR_COUNT);

    translationList[0].language = translationList[1].language = std::string("0409");
    translationList[2].language = translationList[3].language = std::string("0000");
    translationList[0].codePage = translationList[2].codePage = std::string("04B0");
    translationList[1].codePage = translationList[3].codePage = std::string("04E4");
}

OutputUpdateDB::~OutputUpdateDB() {}

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

    getVariableVersion();
    getFixedVersion();

    return OK;
}

int OutputUpdateDB::getFixedVersion()
{
    if (versionInfo[2] && versionInfo[3])
        return OK;

    LPVOID ptr;
    if (!VerQueryValue((LPVOID)verInfo.get(), TEXT("\\"), &ptr, (PUINT)&verSize))
        return FAIL;

    fixedVerInfo.reset((VS_FIXEDFILEINFO *)ptr);
    std::stringstream str;
    if (versionInfo[2])
    {
        str << HIWORD(fixedVerInfo->dwProductVersionMS) << "." << LOWORD(fixedVerInfo->dwProductVersionMS) << "."
            << HIWORD(fixedVerInfo->dwProductVersionLS) << "." << LOWORD(fixedVerInfo->dwProductVersionLS);
        productVerHelperStr = std::move(str.str());
        versionInfo[2].reset(new char[productVerHelperStr.size()]);
        std::strncpy(versionInfo[2].get(), productVerHelperStr.data(), productVerHelperStr.size());
    }

    str.str("");
    if (!versionInfo[3])
    {
        str << HIWORD(fixedVerInfo->dwFileVersionMS) << "." << LOWORD(fixedVerInfo->dwFileVersionMS) << "."
            << HIWORD(fixedVerInfo->dwFileVersionLS) << "." << LOWORD(fixedVerInfo->dwFileVersionLS);
        fileVerHelperStr = std::move(str.str());
        versionInfo[3].reset(new char[fileVerHelperStr.size()]);
        std::strncpy(versionInfo[2].get(), fileVerHelperStr.data(), fileVerHelperStr.size());
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

int OutputUpdateDB::getVariableVersion()
{
    bool foundResult = false;
    std::stringstream str;
    for (auto &trnsltion : translationList)
    {
        str << "\\StringFileInfo\\" << trnsltion.language << trnsltion.codePage << "\\";

        for (int j = 0; j < VERSION_INFO_ATTR_COUNT; j++)
        {
            if (VerQueryValue((LPVOID)verInfo.get(), str.str().append(versionAttributes[j]).data(),
                              (LPVOID *)&verStr, (PUINT)&verSize))
                versionInfo[j].reset(verStr);
            foundResult = versionInfo[j] ? true : false;
        }
        if (foundResult)
            break;
    }

    return !foundResult;
}

int OutputUpdateDB::init()
{
    if (connection.init("INSERT INTO fileinfo"
                        " (file_name, file_created, file_changed, file_registered,"
                        " file_digest, file_type, company_name, product_name,"
                        " product_version, file_version, file_description, os_combination)"
                        " VALUES ('?', UNIX_TIMESTAMP(?), UNIX_TIMESTAMP(?), NOW(),"
                        " '?', '?', '?','?', '?', '?', '?', '?')"))
        return FAIL;
    return OK;
}

int OutputUpdateDB::insertData(std::string &digest, std::string &name)
{
    std::unique_ptr<char[]> digestStr(new char[digest.size()]);
    std::unique_ptr<char[]> nameStr(new char[name.size()]);
    std::unique_ptr<char[]> typeStr(new char[fileType.size()]);

    std::strncpy(digestStr.get(), digest.data(), digest.size());
    std::strncpy(nameStr.get(), name.data(), name.size());
    std::strncpy(typeStr.get(), fileType.data(), fileType.size());

    for (auto &info : versionInfo)
    {
        if (!info)
        {
            info.reset(new char[sizeof(char)]);
            info.get()[0] = '\0';
        }
    }

    int rc = connection.executeInsert(
        nameStr.get(), buffer.st_ctime, buffer.st_mtime, digestStr.get(), typeStr.get(), versionInfo);

    for (auto &info : versionInfo)
        info.reset();
    productVerHelperStr.clear();
    fileVerHelperStr.clear();
    fileType = "Unknown";

    return rc;
}

int OutputUpdateDB::outputData(std::string &digest, std::string &name)
{
    getFileInfo(name);
    return insertData(digest, name);
}

int OutputUpdateDB::printErr(int errNum, const char *errInfo)
{
    printFailed(errInfo);
    std::cerr << " - " << strerror(errNum) << "\n";
    return 1;
}

#endif