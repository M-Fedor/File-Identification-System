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
#include <stringapiset.h>

#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif
#endif

/* Definitions of simple general utilities not specific
 to any component */

int printFailed(const char *errInfo);
int printFailed(const std::ostringstream &errInfo);
int printFailed(const std::wostringstream &errInfo);
void printGreen(const char *str);
void printOrange(const char *str);
void printRed(const char *str);
void printWarning(const char *warnInfo);
void printWarning(const std::ostringstream &warnInfo);
void resetCursor();

#if defined(_WIN32)
void printColorized(const char *str, int color);
std::wstring MultiByteToUTF16(std::string inStr, int encoding);
std::string UTF16ToMultiByte(std::wstring inStr, int encoding);
#endif

#endif
