#include "InputFile.h"

/* Constructor */
InputFile::InputFile(const char *name, const char *pattern)
    : srcFileName(name), bufferSizeFactor(1)
{
    fileDigest = new char[DIGEST_SIZE];
    fileName = new char[IN_NAME_SIZE];
    regex = pattern ? std::regex(pattern) : std::regex(".*");
}

/* Destructor */
InputFile::~InputFile()
{
    fInput.clear(std::_S_goodbit);
    fInput.close();
    if (fInput.fail())
        std::cerr << "\033[31mFAILED\033[0m to close file\033[1m"
                  << srcFileName << "\033[0m\n";

    delete[] fileDigest;
    delete[] fileName;
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
    bool truncated = false;

    do
    {
        fInput.getline(fileName, IN_NAME_SIZE * bufferSizeFactor);
        if (fInput.fail() && !fInput.eof())
        {
            resizeBuffers();
            fInput.seekg(IN_NAME_SIZE * (-bufferSizeFactor), std::ios_base::cur);
            fInput.clear(std::_S_goodbit);
            truncated = true;
            continue;
        }
        truncated = false;
        pathName.assign(fileName);

        while (fInput.peek() == '\n')
            pathName.push_back(fInput.get());

        fInput.getline(fileDigest, DIGEST_SIZE);
        digest.assign(fileDigest);
    } while (!std::regex_match(fileName, regex) || truncated);

    return 0;
}

/* Resize buffers for data from input component, allows us to adapt to various conditions */
void InputFile::resizeBuffers()
{
    bufferSizeFactor *= 2;
    delete[] fileName;
    fileName = new char[IN_NAME_SIZE * bufferSizeFactor];
}