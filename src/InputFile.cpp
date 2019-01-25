#include "InputFile.h"

InputFile::InputFile()
{
    fileDigest = new char[DIGEST_SIZE];
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

std::string InputFile::inputDigest()
{
    return digest;
}

int InputFile::inputNextFile(std::string &pathName)
{
    if (feof(fRead))
        return -1;

    fscanf(fRead, "%255[^\n]\n", fileName);
    pathName.assign(fileName);

    if (feof(fRead))
        digest.clear();
    else
    {
        fscanf(fRead, "%255[^\n]\n", fileDigest);
        digest.assign(fileDigest);
    }

    return 0;
}