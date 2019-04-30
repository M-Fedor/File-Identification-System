#include "ParallelExecutor.h"
#include <unistd.h>

int main(int /* argc */, char **argv)
{
    std::string path(argv[1]);

    InputScanner *inScanner = new InputScanner(path);
    std::vector<HashAlgorithm *> hashAlgList;
    std::vector<Output *> outputList;
    for (int i = 0; i < 4; i++)
    {
        hashAlgList.push_back(new SHA2());
        outputList.push_back(new OutputOffline("Offline_scan.txt"));
    }

    ParallelExecutor *exec = new ParallelExecutor(inScanner, hashAlgList, outputList);
    if (exec->init())
        return 1;
    exec->validate();
    delete exec;
    outputList.clear();

    const char *fileName = "Validation_results.txt";
    const char *host = "localhost";
    const char *user = "root";
    const char *passwd = "rootpassword";
    const char *dbName = "test";

    InputFile *inputFile = new InputFile("Offline_scan.txt");
    for (int i = 0; i < 4; i++)
        outputList.push_back(
            new OutputDBConnection(fileName, host, user, passwd, dbName, 3306, NULL));
    exec = new ParallelExecutor(inputFile, outputList);
    if (exec->init())
        return 1;
    exec->validate();
    delete exec;

    return 0;
}