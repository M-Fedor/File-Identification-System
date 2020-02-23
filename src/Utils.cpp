#include "Utils.h"

/* Print error message using common format */
int printFailed(const char *errInfo)
{
    std::cerr << errInfo << " - ";
    printRed("FAILED\n");
    return FAIL;
}

/* Print error message using common format */
int printFailed(const std::ostringstream &errInfo)
{
    std::cerr << errInfo.str() << " - ";
    printRed("FAILED\n");
    return FAIL;
}

/* Print error message using common format */
int printFailed(const std::wostringstream &errInfo)
{
    std::wcerr << errInfo.str() << " - ";
    printRed("FAILED\n");
    return FAIL;
}

/* Helper function used to configure Windws terminal 
with proper color parameter */
#if defined(_WIN32)
void printColorized(const char *str, int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    std::cerr << str;
    SetConsoleTextAttribute(hConsole, WHITE);
}
#endif

/* Print str in green color based on platform currently in use */
void printGreen(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[32m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, GREEN);
#endif
}

/* Print str in orange color based on platform currently in use */
void printOrange(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[33m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, YELLOW);
#endif
}

/* Print str in red color based on platform currently in use */
void printRed(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[31m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, RED);
#endif
}

/* Print warning message using common format */
void printWarning(const char *warnInfo)
{
    std::cerr << warnInfo << " - ";
    printOrange("WARNING\n");
}

/* Print warning message using common format */
void printWarning(const std::ostringstream &warnInfo)
{
    std::cerr << warnInfo.str() << " - ";
    printOrange("WARNING\n");
}

/* Set cursor to point on the beginning of line above 
current line based on platform currently in use */
void resetCursor()
{
#if defined(__linux__)
    std::cout << "\033[1A\033[30D";
#elif defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD newPosition;

    GetConsoleScreenBufferInfo(hConsole, &info);
    newPosition.X = 0;
    newPosition.Y = info.dwCursorPosition.Y - 1;
    SetConsoleCursorPosition(hConsole, newPosition);
#endif
}

#if defined(_WIN32)
/* Converts UTF8 string to UTF16. As a fallback, on failure assumes an ANSI-Code-Page encoded 
input string in subsequent recursive call. */
std::wstring MultiByteToUTF16(std::string &inStr, const int codePage)
{
    std::wstring outStr;
    if (inStr.empty())
        return outStr;

    int outStrSize = MultiByteToWideChar(
        codePage, MB_ERR_INVALID_CHARS, inStr.data(), inStr.size(), NULL, 0);
    if (!outStrSize)
    {
        if (codePage == CP_UTF8)
            outStr = MultiByteToUTF16(inStr, CP_THREAD_ACP);
        else
            printFailed(static_cast<std::ostringstream &>(
                std::ostringstream() << "Convert " << codePage << " to UTF16: \""
                                     << inStr << "\" (code " << GetLastError() << ")"));
        return outStr;
    }
    outStr.resize(outStrSize);
    if (!MultiByteToWideChar(
            codePage, MB_ERR_INVALID_CHARS, inStr.data(), inStr.size(), &outStr[0], outStrSize))
    {
        printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Convert " << codePage << " to UTF16: \""
                                 << inStr << "\" (code " << GetLastError() << ")"));
        return std::wstring();
    }

    return outStr;
}

/* Converts UTF16 string to UTF8 one. */
std::string UTF16ToUTF8(std::wstring &inStr)
{
    std::string outStr;
    if (inStr.empty())
        return outStr;

    int outStrSize = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, inStr.data(), inStr.size(), NULL, 0, NULL, NULL);
    if (!outStrSize)
    {
        printFailed(static_cast<std::wostringstream &>(
            std::wostringstream() << L"Convert UTF16 to UTF8: \""
                                  << inStr << L"\" (code " << GetLastError() << L")"));
        return outStr;
    }
    outStr.resize(outStrSize);
    if (!WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, inStr.data(), inStr.size(), &outStr[0], outStrSize, NULL, NULL))
    {
        printFailed(static_cast<std::wostringstream &>(
            std::wostringstream() << L"Convert UTF16 to UTF8: \""
                                  << inStr << L"\" (code " << GetLastError() << L")"));
        return std::string();
    }

    return outStr;
}
#endif
