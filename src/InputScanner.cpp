#include "InputScanner.h"

/* Constructor, set root of search as single directory */
InputScanner::InputScanner(std::string &rootDirectory, const char *pattern)
{
    rootDirectories.push_back(rootDirectory);
    regex = std::regex(pattern);
}

/* Constructor, set roots of serach as list of directories */
InputScanner::InputScanner(
    std::vector<std::string> &rootDirectories, const char *pattern)
{
    this->rootDirectories.assign(rootDirectories.begin(), rootDirectories.end());
    regex = std::regex(pattern);
}

/* Destructor */
InputScanner::~InputScanner()
{
    while (!directoryStreams.empty())
    {
        if (closedir(directoryStreams.back()) == -1)
            printErr(errno, static_cast<std::ostringstream &>(
                                std::ostringstream() << "Close directory " << absolutePaths.back().data()));
        directoryStreams.pop_back();
        absolutePaths.pop_back();
    }
}

/* Iterate through file-system, return next file's opened file descriptor for reading,
fill absolute path of the file in pathName, return -1 when no more files can be found
in current root of search */
int InputScanner::findNextFDRec(std::ifstream &fDescriptor, std::string &pathName)
{
    errno = 0; // readdir returns NULL and doesn't change errno on END-OF-DIRECTORY
    struct dirent *dirContent = readdir(directoryStreams.back());

    if (errno == 0) // Did error occur?
    {
        if (dirContent == NULL) // Was the END-OF-DIRECTORY reached?
        {
            if (closedir(directoryStreams.back()) == -1)
                printErr(errno, static_cast<std::ostringstream &>(
                                    std::ostringstream() << "Close directory " << absolutePaths.back().data()));
            directoryStreams.pop_back();
            absolutePaths.pop_back();
            return !directoryStreams.empty() ? findNextFDRec(fDescriptor, pathName) : -1;
        }

        std::string path = absolutePaths.back();
        if (isDirectory(path.append(dirContent->d_name)))
        {
            if (strcmp(dirContent->d_name, ".") && strcmp(dirContent->d_name, ".."))
            {
                DIR *dirStream = opendir(path.append("/").data());
                if (dirStream != NULL)
                {
                    directoryStreams.push_back(dirStream);
                    absolutePaths.push_back(path);
                    return findNextFDRec(fDescriptor, pathName);
                }
                else
                    printErr(errno, static_cast<std::ostringstream &>(
                                        std::ostringstream() << "Open directory " << path.data()));
            }
        }
        else // Is the next item in directory anything else?
        {
            fDescriptor.open(path.data());
            if (fDescriptor.good())
            {
                pathName.assign(path);
                return 0;
            }
            else
                printFailed(static_cast<std::ostringstream &>(
                    std::ostringstream() << "Open file " << path.data()));
        }
    }
    else
        printErr(errno, static_cast<std::ostringstream &>(
                            std::ostringstream() << "Read from directory " << absolutePaths.back().data()));
    return -2; // Return -2 on system error
}

/* Try to open next directory from list of search roots,
initialize search of this directory properly */
int InputScanner::init()
{
    if (rootDirectories.size() == 0)
        return -1;

    DIR *dirStream = NULL;
    std::string path;

    do // Do until some directory is opened successfully
    {
        path = rootDirectories.back().append("/");
        dirStream = opendir(path.data());
        if (dirStream == NULL)
            printErr(errno, static_cast<std::ostringstream &>(
                                std::ostringstream() << "Open directory " << rootDirectories.back().data()));
        rootDirectories.pop_back();
    } while (dirStream == NULL && rootDirectories.size() != 0);

    // Push new streams and paths to corresponding lists, so next time those will be observed
    if (dirStream != NULL)
    {
        directoryStreams.push_back(dirStream);
        absolutePaths.push_back(path);
        return 0;
    }
    return -1;
}

/* Iterate through file system until some file is opened successfuly
or END-OF-DIRECTORY is reached */
int InputScanner::inputNextFile(std::ifstream &fDescriptor, std::string &pathName)
{
    bool match = false;
    int rc = -3;
    do
    {
        rc = findNextFDRec(fDescriptor, pathName);
        match = std::regex_match(pathName, regex);
        if (!match && fDescriptor.is_open())
            fDescriptor.close();
    } while (rc == -2 || (!rc && !match));

    while (rc == -1)
    {
        if (init() == -1)
            break;
        do
        {
            rc = findNextFDRec(fDescriptor, pathName);
            match = std::regex_match(pathName, regex);
            if (!match && fDescriptor.is_open())
                fDescriptor.close();
        } while (rc == -2 || (!rc && !match));
    }

    return rc;
}

bool InputScanner::isDirectory(std::string path)
{
#if defined(__linux__)
    stat(path.data(), &buffer);
    return S_ISDIR(buffer.st_mode) ? true : false;
#elif defined(_WIN32)
    return (GetFileAttributesA(path.data()) == FILE_ATTRIBUTE_DIRECTORY) ? true : false;
#endif
}

/* Print error details */
void InputScanner::printErr(int errNum, const std::ostringstream &errInfo)
{
    printFailed(errInfo);
    std::cerr << " - " << strerror(errNum) << "\n";
}
