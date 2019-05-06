#include "ParallelExecutor.h"
#include <unistd.h>

int main(int /* argc */, char **argv)
{
    std::string path(argv[1]);

    InputScanner *inScanner = new InputScanner(path);
    std::vector<HashAlgorithm *> hashAlgList;
    OutputOffline *out = new OutputOffline("Offline_scan.txt");
    for (int i = 0; i < 1; i++)
        hashAlgList.push_back(new SHA2());

    ParallelExecutor *exec = new ParallelExecutor(inScanner, hashAlgList, out);
    if (exec->init())
        return 1;
    exec->validate();

    delete inScanner;
    for (HashAlgorithm *hash : hashAlgList)
        delete hash;
    delete out;
    delete exec;

    const char *host = "localhost";
    const char *user = "root";
    const char *passwd = "rootpassword";
    const char *dbName = "test";

    InputFile *inputFile = new InputFile("Offline_scan.txt");
    out = new OutputOffline("Validation_results.txt");
    std::vector<Output *> outputList;
    for (int i = 0; i < 4; i++)
        outputList.push_back(
            new OutputDBConnection(out, host, user, passwd, dbName, 3306, NULL));
    exec = new ParallelExecutor(inputFile, outputList);
    if (exec->init())
        return 1;
    exec->validate();

    delete inputFile;
    delete out;
    for (Output *out : outputList)
        delete out;
    delete exec;

    return 0;
}