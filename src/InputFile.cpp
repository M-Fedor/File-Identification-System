#include "InputFile.h"

/* Constructor */
InputFile::InputFile(const char *name, const char *pattern)
    : srcFileName(name) { regex = std::regex(pattern); }

/* Destructor */
InputFile::~InputFile()
{
    fInput.clear(std::_S_goodbit);
    fInput.close();
    if (fInput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close file\033[1m"
                  << srcFileName << "\033[0m\n";
}

/* Initialize source file reader and report any failures */
int InputFile::init()
{
    fInput.open(srcFileName);
    if (fInput.fail())
    {
        std::cerr << "\033[31mFAILED\033[0m to open \033[1m"
                  << srcFileName << "\033[0m\n";
        return 1;
    }

    return 0;
}

/* Load next file's credentials, fill absolute path of the file and 
corresponding file identifier in pathname and digest respectively,
return -1 when no more files are in the list */
int InputFile::inputNextFile(std::string &digest, std::string &pathName)
{
    if (fInput.peek() == EOF)
        return -1;

    do
    {
        std::getline(fInput, pathName);
        while (fInput.peek() == '\n')
            pathName.push_back(fInput.get());

        std::getline(fInput, digest);
    } while (!std::regex_match(digest, regex));

    return 0;
}