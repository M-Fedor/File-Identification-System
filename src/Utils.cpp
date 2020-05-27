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
    SetConsoleTextAttribute(hConsole, (WORD)color);
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
/* Converts ANSI-Code-Page encoded string to UTF16. */
std::wstring MultiByteToUTF16(std::string inStr, int encoding)
{
    std::wstring outStr;
    if (inStr.empty())
        return outStr;

    int outStrSize = MultiByteToWideChar(
        encoding, MB_ERR_INVALID_CHARS, inStr.data(), inStr.size(), NULL, 0);
    if (!outStrSize)
    {
        std::ostringstream stream;
        stream << "Convert MultiByte to UTF16: \"" << inStr << "\" (code " << GetLastError() << ")";
        printFailed(stream);
        return outStr;
    }
    outStr.resize(outStrSize);
    if (!MultiByteToWideChar(
            encoding, MB_ERR_INVALID_CHARS, inStr.data(), inStr.size(), &outStr[0], outStrSize))
    {
        std::ostringstream stream;
        stream << "Convert MultiByte to UTF16: \"" << inStr << "\" (code " << GetLastError() << ")";
        printFailed(stream);
        return std::wstring();
    }

    return outStr;
}

/* Converts UTF16 encoded string to UTF8 one. */
std::string UTF16ToMultiByte(std::wstring inStr, int encoding)
{
    std::string outStr;
    if (inStr.empty())
        return outStr;

    int outStrSize = WideCharToMultiByte(
        encoding, (encoding == CP_UTF8) ? WC_ERR_INVALID_CHARS : 0, inStr.data(), inStr.size(), NULL, 0, NULL, NULL);
    if (!outStrSize)
    {
        std::wostringstream stream;
        stream << L"Convert UTF16 to MultiByte: \"" << inStr << L"\" (code " << GetLastError() << L")";
        printFailed(stream);
        return outStr;
    }
    outStr.resize(outStrSize);
    if (!WideCharToMultiByte(
            encoding, (encoding == CP_UTF8) ? WC_ERR_INVALID_CHARS : 0, inStr.data(), inStr.size(), &outStr[0], outStrSize, NULL, NULL))
    {
        std::wostringstream stream;
        stream << L"Convert UTF16 to MultiByte: \"" << inStr << L"\" (code " << GetLastError() << L")";
        printFailed(stream);
        return std::string();
    }

    return outStr;
}
#endif
