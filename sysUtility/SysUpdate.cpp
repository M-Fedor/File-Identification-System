#include "SysUpdate.h"

int main()
{
	SysUpdate update;
	update.init();
	update.listUpdates();

	return OK;
}

SysUpdate::SysUpdate() : uListLoaded(false) {}
SysUpdate::~SysUpdate() { CoUninitialize(); }

const wchar_t *SysUpdate::enumerateInstallImpact(InstallationImpact impact)
{
	switch (impact)
	{
	case 0:
		return L"Normal";
	case 1:
		return L"Minor";
	case 2:
		return L"Requires Exclusive Handling";
	default:
		return L"Invalid Value";
	}
}

const wchar_t *SysUpdate::enumerateRebootBehaviour(InstallationRebootBehavior reboot)
{
	switch (reboot)
	{
	case 0:
		return L"Never Reboots";
	case 1:
		return L"Always Requires Reboot";
	case 2:
		return L"Can Request Reboot";
	default:
		return L"Invalid Value";
	}
}

int SysUpdate::init()
{
	res = CoInitialize(NULL);
	if (res != S_OK)
		return printFailed("Initialize COM library");

	res = CoCreateInstance(CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (void **)&uSession);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateSession");

	res = uSession->CreateUpdateSearcher(&uSearcher);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateSearcher");

	res = uSession->CreateUpdateDownloader(&uDownloader);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateDownloader");

	res = uSession->CreateUpdateInstaller(&uInstaller);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateInstaller");

	return OK;
}

int SysUpdate::listUpdates()
{
	if (!uListLoaded)
	{
		if (searchUpdates())
			return FAIL;
		uListLoaded = true;
	}

	uCollection->get_Count(&uCount);
	for (LONG i = 0; i < uCount; i++)
	{
		uCollection->get_Item(i, &uItem);
		printUpdateInfo(uItem, std::wstring());
	}

	return OK;
}

int SysUpdate::printErr(HRESULT errCode, const char *errInfo)
{
	printFailed(errInfo);
	std::cerr << std::system_category().message(errCode);
	return FAIL;
}

int SysUpdate::printUpdateInfo(IUpdate *item, std::wstring indent)
{
	BSTR title;
	item->get_Title(&title);
	std::wcout << indent << L"Title: " << title << L"\n";

	UpdateType type;
	item->get_Type(&type);
	std::wcout << indent << L"Type: " << ((type == 1) ? L"software" : L"driver") << L"\n";

	BSTR severity;
	item->get_MsrcSeverity(&severity);
	std::wcout << indent << L"Severity: " << (SysStringLen(severity) ? severity : L"") << L"\n";

	BSTR description;
	item->get_Description(&description);
	std::wcout << indent << L"Description: " << (SysStringLen(description) ? description : L"") << L"\n";

	IUpdateIdentity *uIdentity;
	item->get_Identity(&uIdentity);
	LONG revisionNum;
	uIdentity->get_RevisionNumber(&revisionNum);
	std::wcout << indent << L"Revision number: " << revisionNum << L"\n";

	IInstallationBehavior *installBehav;
	InstallationImpact impact;
	InstallationRebootBehavior reboot;
	item->get_InstallationBehavior(&installBehav);
	installBehav->get_Impact(&impact);
	installBehav->get_RebootBehavior(&reboot);
	std::wcout << indent << L"Instalation impact: " << enumerateInstallImpact(impact) << L"\n"
			   << indent << L"Instalation reboot behaviour: " << enumerateRebootBehaviour(reboot) << L"\n";

	IStringCollection *strCollection;
	LONG stringCount;
	item->get_KBArticleIDs(&strCollection);
	strCollection->get_Count(&stringCount);
	for (LONG i = 0; i < stringCount; i++)
	{
		BSTR str;
		strCollection->get_Item(i, &str);
		std::wcout << indent << L"ArticleID: " << (SysStringLen(str) ? str : L"") << L"\n";
	}

	IUpdateCollection *uCollection;
	LONG uCount;
	item->get_BundledUpdates(&uCollection);
	uCollection->get_Count(&uCount);
	std::wcout << indent << L"Contains bundled updates: " << uCount << L"\n\n";

	if (uCount)
	{
		std::wcout << indent << L"//////// BUNDLED UPDATE LIST START ////////" << L"\n\n";
		for (LONG i = 0; i < uCount; i++)
		{
			uCollection->get_Item(i, &item);
			printUpdateInfo(item, std::wstring(indent).append(INDENT));
		}
		std::wcout << indent << L"//////// BUNDLED UPDATE LIST END ////////" << L"\n\n";
	}

	return OK;
}

int SysUpdate::searchUpdates()
{
	uSearcher->put_IncludePotentiallySupersededUpdates(VARIANT_TRUE);
	uSearcher->put_Online(VARIANT_TRUE);

	BSTR criteria = SysAllocString(L"IsInstalled=0");
	res = uSearcher->Search(criteria, &searchRes);
	if (res != S_OK)
		return printErr(res, "Search for updates");

	searchRes->get_Updates(&uCollection);
	return OK;
}
