#include "InputFile.h"

/* Constructor */
InputFile::InputFile(char *name)
{
    fileDigest = new char[DIGEST_SIZE];
    fileName = new char[NAME_SIZE];
    srcFileName = name;
}

/* Destructor */
InputFile::~InputFile()
{
    fInput.clear(std::_S_goodbit);
    fInput.close();
    if (fInput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close \033[1m"
                  << srcFileName << "\033[0m\n";

    delete[] fileDigest;
    delete[] fileName;
    free(srcFileName);
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

/* Return file's corresponding unique identifier */
std::string InputFile::inputDigest() { return digest; }

/* Load next file's credentials, fill absolute path of the file
in pathName, set the digest value on corresponding file identifier,
return -1 when no more files are in the list */
int InputFile::inputNextFile(std::string &pathName)
{
    if (fInput.peek() == EOF)
        return -1;

    fInput.getline(fileName, NAME_SIZE);
    pathName.assign(fileName);

    while (fInput.peek() == '\n')
        pathName.push_back(fInput.get());

    fInput.getline(fileDigest, DIGEST_SIZE);
    digest.assign(fileDigest);

    return 0;
}