#ifndef OutputUpdateDB_h
#define OutputUpdateDB_h

#if defined(_WIN32)
#define VERSION_INFO_ATTR_COUNT 6
#define TRNSLTION_COUNT 4

#include "DBConnection.h"
#include "Output.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <versionhelpers.h>
#include <winver.h>

/* Class implements the way to find out useful legitimate information 
about user-specified files from file system and update reference database 
using the information on Microsoft Windows platform */
class OutputUpdateDB : public Output
{
public:
    OutputUpdateDB(DBConnection &conn);
    ~OutputUpdateDB();

    int init();
    int outputData(std::string &digest, std::string &name);

private:
    /* Structure contains string representation of hexadecimal
    language and code-page codes for navigation in VersionInfo
    String-tables */
    struct Translation
    {
        std::string language;
        std::string codePage;
    };

    int getFileInfo(std::string &name);
    int getFixedVersion();
    int getOSVersion();
    int getType();
    int getVariableVersion();
    int insertData(std::string &digest, std::string &name);
    int printErr(int errNum, const std::ostringstream &errInfo);

    DBConnection connection;
    LPSTR verStr;
    size_t verSize;
    std::string fileType;
    std::string fileVerHelperStr;
    std::string productVerHelperStr;
    std::shared_ptr<char[]> defaultStr;
    std::unique_ptr<char[]> osVerStr;
    std::unique_ptr<char[]> verInfo;
    std::vector<LPCSTR> versionAttributes;
    std::vector<char *> versionInfo;
    struct Translation translationList[TRNSLTION_COUNT];
    struct _stat64 buffer;
    VS_FIXEDFILEINFO *fixedVerInfo;
};

#endif
#endif
