#include "OutputOffline.h"

/* Constructor */
OutputOffline::OutputOffline(char *fileName)
{
    this->fileName = fileName;
}

/* Destructor */
OutputOffline::~OutputOffline()
{
    fOutput.clear(std::_S_goodbit);
    fOutput.close();
    if (fOutput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close \033[1m"
                  << fileName << "\033[0m\n";
    free(fileName);
}

/* Open output file, report any failures */
int OutputOffline::init()
{
    fOutput.open(fileName, std::ios::trunc);
    if (fOutput.fail())
    {
        std::cerr << "\033[31mFAILED\033[0m to open \033[1m"
                  << fileName << "\033[0m\n";
        return 1;
    }

    return 0;
}

int OutputOffline::outputData(std::string data)
{
    mutex.lock();
    fOutput << data.data();
    mutex.unlock();

    return 0;
}

/* Create formatted output */
int OutputOffline::outputData(std::string digest, std::string name)
{
    mutex.lock();
    fOutput << name.data() << "\n"
            << digest.data() << "\n";
    mutex.unlock();

    return 0;
}