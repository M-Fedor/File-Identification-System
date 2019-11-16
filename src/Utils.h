#ifndef Utils_h
#define Utils_h

#include <iostream>
#include <sstream>

#if defined(_WIN32)
#define BLACK 0
#define GREEN 2
#define RED 4
#define YELLOW 6

#include <windows.h>
#endif

void printFailed(const char *errInfo);
void printFailed(const std::ostringstream &errInfo);
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