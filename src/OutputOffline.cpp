#include "OutputOffline.h"

/* Constructor */
OutputOffline::OutputOffline() {}

/* Destructor */
OutputOffline::~OutputOffline()
{
    if (fclose(fWrite))
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to close \033[1mOffline_scan.txt\033[0m\n");
        perror(strerror(temp_errno));
    }
}

/* Open output file, report any failures */
int OutputOffline::init()
{
    fWrite = fopen("Offline_scan.txt", "w");
    if (!fWrite)
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to open \033[1mOffline_scan.txt\033[0m\n");
        perror(strerror(temp_errno));
        return 1;
    }

    return 0;
}

/* Create formatted output */
int OutputOffline::outputData(std::string digest, std::string name)
{
    fprintf(fWrite, "%s\n%s\n", name.data(), digest.data());
    return 0;
}