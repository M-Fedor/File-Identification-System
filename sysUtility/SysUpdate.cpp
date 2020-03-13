#include "SysUpdate.h"

#if defined(_WIN32)
int main()
{
	SysUpdate update(true);

	if (update.init())
		return FAIL;
	if (update.update())
		return FAIL;

	return OK;
}

SysUpdate::SysUpdate(bool verbose) : verbose(verbose) {}
SysUpdate::~SysUpdate() { CoUninitialize(); }

int SysUpdate::downloadUpdates()
{
	IDownloadResult *downloadRes;
	OperationResultCode resultCode;

	uDownloader->put_Updates(uCollection);
	res = uDownloader->Download(&downloadRes);
	if (res != S_OK)
		return printErr(res, "Download updates");

	downloadRes->get_ResultCode(&resultCode);
	std::wcout << L"Download result: " << enumerateResultCode(resultCode) << L"\n\n";

	if (resultCode == orcSucceededWithErrors)
	{
		LONG uCount;
		IUpdateDownloadResult *updateRes;

		std::cout << "//////// FAILED UPDATE DOWNLOAD ////////\n\n";

		uCollection->get_Count(&uCount);
		for (LONG i = 0; i < uCount; i++)
		{
			downloadRes->GetUpdateResult(i, &updateRes);
			updateRes->get_ResultCode(&resultCode);
			if (resultCode == orcFailed)
			{
				uCollection->get_Item(i, &uItem);
				printUpdateInfo(uItem, std::wstring());
			}
		}
	}

	return OK;
}

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

const wchar_t *SysUpdate::enumerateResultCode(OperationResultCode code)
{
	switch (code)
	{
	case 2:
		return L"Succeded";
	case 3:
		return L"Succeded with errors";
	case 4:
		return L"Failed";
	case 5:
		return L"Aborted";
	default:
		return L"Invalid value";
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

	res = CoCreateInstance(CLSID_UpdateInstaller, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateInstaller2, (void **)&uInstaller);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateSession");

	res = CoCreateInstance(CLSID_UpdateCollection, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateCollection, (void **)&uCollection);
	if (res != S_OK)
		return printErr(res, "Create instance of IUpdateCollection");

	currentIndents.push_back(std::wstring());
	currentPositions.push_back(0);

	return OK;
}

int SysUpdate::installUpdates()
{
	IInstallationResult *installRes;
	OperationResultCode resultCode;
	VARIANT_BOOL rebootReq;

	uInstaller->put_ForceQuiet(VARIANT_TRUE);
	uInstaller->put_Updates(uCollection);
	res = uInstaller->Install(&installRes);
	if (res != S_OK)
		return printErr(res, "Download updates");

	installRes->get_ResultCode(&resultCode);
	installRes->get_RebootRequired(&rebootReq);
	std::wcout << L"Instalation result: " << enumerateResultCode(resultCode) << L"\n"
			   << L"Reboot required: " << ((!rebootReq) ? L"No" : L"Yes") << L"\n\n";

	if (resultCode == orcSucceededWithErrors)
	{
		LONG uCount;
		IUpdateInstallationResult *updateRes;

		std::cout << "//////// FAILED UPDATE INSTALLATION ////////\n\n";

		uCollection->get_Count(&uCount);
		for (LONG i = 0; i < uCount; i++)
		{
			installRes->GetUpdateResult(i, &updateRes);
			updateRes->get_ResultCode(&resultCode);
			if (resultCode == orcFailed)
			{
				uCollection->get_Item(i, &uItem);
				printUpdateInfo(uItem, std::wstring());
			}
		}
	}

	return (!rebootReq) ? OK : REBOOT;
}

int SysUpdate::listNextUpdateItem()
{
	IUpdateCollection *coll;
	LONG bundledCount;

	collections.back()->get_Count(&bundledCount);
	if (currentPositions.back() == bundledCount)
	{
		collections.pop_back();
		currentIndents.pop_back();
		currentPositions.pop_back();

		std::wcout << ((currentIndents.size()) ? currentIndents.back() : L"")
				   << L"//////// BUNDLED UPDATE LIST END ////////\n\n";

		return (collections.size()) ? listNextUpdateItem() : UNDEFINED;
	}

	collections.back()->get_Item(currentPositions.back(), &uItem);
	uItem->get_BundledUpdates(&coll);
	coll->get_Count(&bundledCount);
	currentPositions.back()++;
	printUpdateInfo(uItem, currentIndents.back());

	if (bundledCount)
	{
		std::wcout << currentIndents.back() << L"//////// BUNDLED UPDATE LIST START ////////\n\n";

		collections.push_back(coll);
		currentPositions.push_back(0);
		std::wstring indentStr = currentIndents.back();
		currentIndents.push_back(indentStr.append(L"\t\t"));
		return listNextUpdateItem();
	}
	else
		return OK;
}

int SysUpdate::printErr(HRESULT errCode, const char *errInfo)
{
	printFailed(errInfo);
	std::cerr << std::system_category().message(errCode) << " " << std::hex << errCode << "\n";
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
	LONG revisionNum;
	item->get_Identity(&uIdentity);
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
	LONG count;
	item->get_KBArticleIDs(&strCollection);
	strCollection->get_Count(&count);
	for (LONG i = 0; i < count; i++)
	{
		BSTR str;
		strCollection->get_Item(i, &str);
		std::wcout << indent << L"ArticleID: " << (SysStringLen(str) ? str : L"") << L"\n";
	}

	IUpdateCollection *coll;
	item->get_BundledUpdates(&coll);
	coll->get_Count(&count);
	std::wcout << indent << L"Contains bundled updates: " << count << L"\n\n";

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
	collections.push_back(uCollection);

	return OK;
}

int SysUpdate::update()
{
	std::cout << "Searching for updates...\n\n";
	if (searchUpdates())
		return FAIL;

	if (verbose)
		while (listNextUpdateItem() != UNDEFINED)
			;

	std::cout << "Downloading updates...\n";
	if (downloadUpdates())
		return FAIL;

	std::cout << "Installing updates...\n";
	int rc = installUpdates();

	return rc;
}
#endif
