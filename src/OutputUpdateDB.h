#ifndef OutputUpdateDB_h
#define OutputUpdateDB_h

#if defined(_WIN32)
#define VERSION_INFO_ATTR_COUNT 6
#define TRNSLTION_COUNT 4

#include "DBConnection.h"
#include "Output.h"
#include "OutputOffline.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <versionhelpers.h>
#include <winver.h>

class OutputUpdateDB : public Output
{
public:
    OutputUpdateDB(DBConnection &conn);
    ~OutputUpdateDB();

    int init();
    int outputData(std::string &digest, std::string &name);

private:
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
    int printErr(int errNum, const char *errInfo);

    DBConnection connection;
    LPSTR verStr;
    size_t verSize;
    std::string fileType;
    std::string fileVerHelperStr;
    std::string productVerHelperStr;
    std::unique_ptr<char[]> defaultStr;
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
