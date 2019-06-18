#ifndef FSS_h
#define FSS_h

#include "ParallelExecutor.h"
#include <getopt.h>
#include <termios.h>
#include <unistd.h>

int execute(ParallelExecutor *exec);
int executeInFileMode();
int executeInScannerMode();
void getInputOpt();
void getOutputOpt();
void printHelp();
void printVersion();
int resolveOptions(int argc, char **args);
void secureInput(std::string &input);

bool inputFile = false;
bool offline = false;
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
