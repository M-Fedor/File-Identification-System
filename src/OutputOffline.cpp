#include "OutputOffline.h"

std::mutex OutputOffline::mutex;

/* Constructor */
OutputOffline::OutputOffline(const char *fileName)
    : fileName(fileName) {}

/* Destructor */
OutputOffline::~OutputOffline()
{
    fOutput.clear(std::_S_goodbit);
    fOutput.close();
    if (fOutput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close \033[1m"
                  << fileName << "\033[0m\n";
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

int OutputOffline::outputData(std::string &data)
{
    std::lock_guard<std::mutex> lock(mutex);
    fOutput << data.data();
    fOutput.flush();
    return 0;
}

/* Create formatted output */
int OutputOffline::outputData(std::string &digest, std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    fOutput << name.data() << "\n"
            << digest.data() << "\n";
    fOutput.flush();
    return 0;
}