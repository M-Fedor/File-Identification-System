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
        printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Close file " << srcFileName));
}

/* Initialize source file reader and report any failures */
int InputFile::init()
{
    fInput.open(srcFileName);
    if (fInput.fail())
        return printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Open " << srcFileName));

    return OK;
}

/* Load next file's credentials, fill absolute path of the file and 
corresponding file identifier in pathname and digest respectively,
return UNDEFINED when no more files are in the list */
int InputFile::inputNextFile(std::string &digest, std::string &pathName)
{
    if (fInput.peek() == EOF)
        return UNDEFINED;

    do
    {
        std::getline(fInput, pathName);
        while (fInput.peek() == '\n')
            pathName.push_back(fInput.get());

        std::getline(fInput, digest);
    } while (!std::regex_match(digest, regex));

    return OK;
}
