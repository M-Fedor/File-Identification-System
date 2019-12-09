#ifndef FSS_h
#define FSS_h

#include "ParallelExecutor.h"
#include <unistd.h>

#if defined(__linux__)
#include <getopt.h>
#include <termios.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

int execute(ParallelExecutor *exec);
int executeInFileMode();
int executeInScannerMode();
void getInputOpt();
void getOutputOpt();
void printHelp();
void printVersion();
int resolveOptionsUnix(int argc, char **args);
int resolveOptionsWin(int argc, char **args);
void secureInput(std::string &input);

bool inputFile = false;
bool offline = false;
bool update = false;
bool verbose = false;
std::string dbName;
std::string errFileName;
std::string hostName;
std::string userName;
std::string password;
std::string inputFileName;
std::string outputFileName;
std::string regexTarget;
std::vector<std::string> rootDirectories;
unsigned int dbPort;
unsigned int nCores;

#endif
