#ifndef SysUpdate_h
#define SysUpdate_h

#define INDENT L"\t\t"

#include "../src/Utils.h"
#include <combaseapi.h>
#include <OleAuto.h>
#include <system_error>
#include <wuapi.h>

class SysUpdate
{
public:
    SysUpdate();
    ~SysUpdate();

    int init();
    int installAllUpdates();
    int installNextUpdate();
    int listUpdates();

private:
    const wchar_t *enumerateInstallImpact(InstallationImpact impact);
    const wchar_t *enumerateRebootBehaviour(InstallationRebootBehavior reboot);
    int printErr(HRESULT errCode, const char *errInfo);
    int printUpdateInfo(IUpdate *item, std::wstring indent);
    int searchUpdates();

    bool uListLoaded;
    HRESULT res;
    ISearchResult *searchRes;
    IUpdateCollection *uBundled;
    IUpdateCollection *uCollection;
    IUpdateDownloader *uDownloader;
    IUpdateInstaller *uInstaller;
    IUpdate *uItem;
    IUpdateSearcher *uSearcher;
    IUpdateSession *uSession;
    LONG bundledCount;
    LONG uCount;
};

#endif
