#ifndef SysUpdate_h
#define SysUpdate_h

#if defined(_WIN32)
#define INDENT L"\t\t"
#define REBOOT 2

#include "../src/Utils.h"
#include <combaseapi.h>
#include <OleAuto.h>
#include <system_error>
#include <vector>
#include <wuapi.h>

class SysUpdate
{
public:
    SysUpdate(bool verbose = true);
    ~SysUpdate();

    int init();
    int update();

private:
    int downloadUpdates();
    const wchar_t *enumerateInstallImpact(InstallationImpact impact);
    const wchar_t *enumerateRebootBehaviour(InstallationRebootBehavior reboot);
    const wchar_t *enumerateResultCode(OperationResultCode code);
    int installUpdates();
    bool isEmptyCollection(IUpdateCollection *coll);
    int listNextUpdateItem();
    int printErr(HRESULT errCode, const char *errInfo);
    int printUpdateInfo(IUpdate *item, std::wstring indent);
    int searchUpdates();

    bool verbose;
    HRESULT res;
    ISearchResult *searchRes;
    IUpdate *uItem;
    IUpdateCollection *uCollection;
    IUpdateCollection *uCollectionExclusive;
    IUpdateDownloader *uDownloader;
    IUpdateInstaller2 *uInstaller;
    IUpdateSearcher *uSearcher;
    IUpdateSession *uSession;
    std::vector<IUpdateCollection *> collections;
    std::vector<LONG> currentPositions;
    std::vector<std::wstring> currentIndents;
};

#endif
#endif
