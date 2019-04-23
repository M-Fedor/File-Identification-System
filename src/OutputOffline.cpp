#include "OutputOffline.h"

/* Constructor */
OutputOffline::OutputOffline() {}

/* Destructor */
OutputOffline::~OutputOffline()
{
    fOutput.close();
    if (fOutput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close"
                     " \033[1mOffline_scan.txt\033[0m\n";
}

/* Open output file, report any failures */
int OutputOffline::init()
{
    fOutput.open("Offline_scan.txt", std::ios::trunc);
    if (fOutput.fail())
    {
        std::cerr << "\033[31mFAILED\033[0m to open"
                     " \033[1mOffline_scan.txt\033[0m\n";
        return 1;
    }

    return 0;
}

/* Create formatted output */
int OutputOffline::outputData(std::string digest, std::string name)
{
    fOutput << name.data() << "\n"
            << digest.data() << "\n";

    return 0;
}