#ifndef Utils_h
#define Utils_h

#define FAIL 1
#define OK 0
#define UNDEFINED -1
#define END -1

#include <iostream>
#include <sstream>

#if defined(_WIN32)
#define GREEN 2
#define RED 4
#define YELLOW 6
#define WHITE 7

#include <windows.h>
#endif

int printFailed(const char *errInfo);
int printFailed(const std::ostringstream &errInfo);
void printGreen(const char *str);
void printOrange(const char *str);
void printRed(const char *str);
void printWarning(const char *warnInfo);
void printWarning(const std::ostringstream &warnInfo);
void resetCursor();

#if defined(_WIN32)
void printColorized(const char *str, int color);
#endif

#endif
