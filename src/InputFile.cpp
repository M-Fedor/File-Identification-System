#include "InputFile.h"

InputFile::InputFile() : currentPos(0)
{
    digest = new char[DIGEST_SIZE];
    fileName = new char[NAME_SIZE];
}

InputFile::~InputFile()
{
    delete[] fileDigest;
    delete[] fileName;

    if (fclose(fRead))
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to close \033[1mOffline_scan.txt\033[0m\n");
        perror(strerror(temp_errno));
    }
}

std::string InputFile::getNextDigest()
{
    if (currentPos == ftell(fRead))
        return digest;
    else if (feof(fRead) == EOF)
    {
        digest.clear();
        return digest;
    }
    else
    {
        currentPos = ftell(fRead);
        fscanf(fRead, "%s", fileDigest);
        digest.assign(fileDigest);
        return digest;
    }
}

int InputFile::getNextFD(std::string &pathName)
{
    int fd = -2;

    do
    {
        if (feof(fRead) == EOF)
            return -1;

        fscanf(fRead, "%s", fileName);
        fd = open(fileName, O_RDONLY);

        if (fd != -1)
        {
            pathName.assign(fileName);
            return fd;
        }
        else
        {
            int temp_errno = errno;
            printf("\033[31mFAILED\033[0m to open file\033[1m %s\033[0m\n", fileName);
            perror(strerror(temp_errno));
            fscanf(fRead, "%s", fileDigest);
        }
    } while (fd == -1);

    return fd;
}

int InputFile::init()
{
    fRead = fopen("Offline_scan.txt", "r");
    if (!fRead)
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to open \033[1mOffline_scan.txt\033[0m\n");
        perror(strerror(temp_errno));
        return 1;
    }

    return 0;
}