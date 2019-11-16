#include "OutputOffline.h"

std::mutex OutputOffline::mutex;

/* Constructor */
OutputOffline::OutputOffline(const char *fileName)
    : fileName(fileName) {}

/* Destructor */
OutputOffline::~OutputOffline()
{
    fOutput.clear(std::_S_goodbit);
    if (fOutput.is_open())
        fOutput.close();
    if (fOutput.fail())
        printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Close file " << fileName));
}

/* Open output file, report any failures */
int OutputOffline::init()
{
    if (!fOutput.is_open())
        fOutput.open(fileName, std::ios::trunc);
    if (fOutput.fail())
    {
        printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Open " << fileName));
        return 1;
    }

    return 0;
}

/* Output data as they are */
int OutputOffline::outputData(std::string &data)
{
    std::lock_guard<std::mutex> lock(mutex);
    fOutput << data;
    fOutput.flush();
    return 0;
}

/* Create formatted output */
int OutputOffline::outputData(std::string &digest, std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    fOutput << name << "\n"
            << digest << "\n";
    fOutput.flush();
    return 0;
}
