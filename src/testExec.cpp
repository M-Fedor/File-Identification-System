#define BUFFER_SIZE 2000

#include "InputFile.h"
#include "InputScanner.h"
#include "OutputDBConnection.h"
#include "OutputOffline.h"
#include "SHA2.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    std::ifstream fDescriptor;
    std::string path(argv[1]);
    std::string digest;
    std::string pathName;
    char *buffer = new char[BUFFER_SIZE];
    int rc = -2;

    Input *inputScanner = new InputScanner(path);
    HashAlgorithm *hashAlg = new SHA2();
    Output *outputOffline = new OutputOffline(
        strdup("Offline_scan.txt"));

    if (inputScanner->init())
        return 1;
    if (outputOffline->init())
        return 1;

    do
    {
        rc = inputScanner->inputNextFile(fDescriptor, pathName);
        std::cout << "Return code: " << rc << "\n";
        if (rc != -1)
        {
            while (fDescriptor.good())
            {
                fDescriptor.read(buffer, BUFFER_SIZE);
                hashAlg->inputDataPart(buffer, BUFFER_SIZE);
            }
            digest = hashAlg->hashData();
            std::cout << digest.data() << "\n";
            fDescriptor.clear(std::_S_goodbit);
            fDescriptor.seekg(0);

            while (fDescriptor.good())
            {
                fDescriptor.read(buffer, BUFFER_SIZE);
                hashAlg->inputDataPart(buffer, BUFFER_SIZE);
            }
            digest = hashAlg->hashData();
            std::cout << digest.data() << "\n";
            fDescriptor.close();

            outputOffline->outputData(digest, pathName);
        }
    } while (rc != -1);

    delete[] buffer;
    delete inputScanner;
    delete hashAlg;
    delete outputOffline;

    char *fileName = strdup("Validation_results.txt");
    char *host = strdup("localhost");
    char *user = strdup("root");
    char *passwd = strdup("rootpassword");
    char *dbName = strdup("test");

    InputFile *inputFile = new InputFile(strdup("Offline_scan.txt"));
    Output *outputDB = new OutputDBConnection(fileName, host, user, passwd, dbName, 3306, NULL);

    if (inputFile->init())
        return 1;
    if (outputDB->init())
        return 1;

    do
    {
        rc = inputFile->inputNextFile(pathName);
        if (rc != -1)
        {
            digest = inputFile->inputDigest();
            outputDB->outputData(digest, pathName);
        }
    } while (rc != -1);

    delete inputFile;
    delete outputDB;

    return 0;
}