#define BUFFER_SIZE 2000

#include "InputFile.h"
#include "InputScanner.h"
#include "OutputDBConnection.h"
#include "OutputOffline.h"
#include "SHA2.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    std::string path(argv[1]);
    std::string digest;
    std::string pathName;
    char *buffer = new char[BUFFER_SIZE];
    int fd = -2;

    Input *inputScanner = new InputScanner(path);
    HashAlgorithm *hashAlg = new SHA2();
    Output *outputOffline = new OutputOffline();

    if (inputScanner->init())
        return 1;
    if (outputOffline->init())
        return 1;

    do
    {
        fd = inputScanner->inputNextFile(pathName);
        printf("FileDescriptor: %d\n", fd);
        if (fd != -1)
        {
            while (read(fd, buffer, BUFFER_SIZE) != 0)
            {
                hashAlg->inputDataPart(buffer, BUFFER_SIZE);
            }
            digest = hashAlg->hashData();
            printf("%s\n", digest.data());
            lseek(fd, 0, SEEK_SET);

            while (read(fd, buffer, BUFFER_SIZE) != 0)
            {
                hashAlg->inputDataPart(buffer, BUFFER_SIZE);
            }
            digest = hashAlg->hashData();
            printf("%s\n", digest.data());
            close(fd);

            outputOffline->outputData(digest, pathName);
        }
    } while (fd != -1);

    delete[] buffer;
    delete inputScanner;
    delete hashAlg;
    delete outputOffline;

    char *host = strdup("localhost");
    char *user = strdup("root");
    char *passwd = strdup("rootpassword");
    char *dbName = strdup("test");

    InputFile *inputFile = new InputFile();
    Output *outputDB = new OutputDBConnection(host, user, passwd, dbName, 3306, NULL);

    if (inputFile->init())
        return 1;
    if (outputDB->init())
        return 1;

    do
    {
        fd = inputFile->inputNextFile(pathName);
        if (fd != -1)
        {
            digest = inputFile->inputDigest();
            outputDB->outputData(digest, pathName);
        }
    } while (fd != -1);

    free(host);
    free(user);
    free(passwd);
    free(dbName);
    delete inputFile;
    delete outputDB;

    return 0;
}