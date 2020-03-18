#ifndef SysUpdate_h
#define SysUpdate_h

#if defined(_WIN32)
#include "SysUpdateImp.h"

int execute();
void printHelp();
void printVersion();
int resolveOptions(int argc, char **args);

bool list = true;
bool update = false;
bool verbose = true;

#endif
#endif
