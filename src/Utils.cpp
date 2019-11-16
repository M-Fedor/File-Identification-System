#include "Utils.h"

void printFailed(const char *errInfo)
{
    std::cerr << errInfo << " - ";
    printRed("FAILED\n");
}

void printFailed(const std::ostringstream &errInfo)
{
    std::cerr << errInfo.str() << " - ";
    printRed("FAILED\n");
}

#if defined(_WIN32)
void printColorized(const char *str, int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    std::cerr << str;
    SetConsoleTextAttribute(hConsole, WHITE);
}
#endif

void printGreen(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[32m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, GREEN);
#endif
}

void printOrange(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[33m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, YELLOW);
#endif
}

void printRed(const char *str)
{
#if defined(__linux__)
    std::cerr << "\033[31m" << str << "\033[0m";
#elif defined(_WIN32)
    printColorized(str, RED);
#endif
}

void printWarning(const char *warnInfo)
{
    std::cerr << warnInfo << " - ";
    printOrange("WARNING\n");
}

void printWarning(const std::ostringstream &warnInfo)
{
    std::cerr << warnInfo.str() << " - ";
    printOrange("WARNING\n");
}

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