#include "InputScanner.h"

/* Constructor, set root of search as single directory */
InputScanner::InputScanner(std::string &rootDirectory, const char *pattern)
{
    rootDirectories.push_back(std::move(rootDirectory));
    regex = std::regex(pattern);
}

/* Constructor, set roots of serach as list of directories */
InputScanner::InputScanner(
    std::vector<std::string> &rootDirectories, const char *pattern)
{
    this->rootDirectories = std::move(rootDirectories);
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
fill absolute path of the file in pathName, return UNDEFINED when no more files can be found
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
            return !directoryStreams.empty() ? findNextFDRec(fDescriptor, pathName) : UNDEFINED;
        }

        std::string path = absolutePaths.back();
        if (isDirectory(path.append(dirContent->d_name)))
        {
            if (strcmp(dirContent->d_name, ".") && strcmp(dirContent->d_name, ".."))
            {
                DIR *dirStream = opendir(path.append(DEFAULT_SEPARATOR).data());
                if (dirStream != NULL)
                {
                    directoryStreams.push_back(dirStream);
                    absolutePaths.push_back(std::move(path));
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
                pathName = std::move(path);
                return OK;
            }
            else
                printFailed(static_cast<std::ostringstream &>(
                    std::ostringstream() << "Open file " << path.data()));
        }
    }
    else
        printErr(errno, static_cast<std::ostringstream &>(
                            std::ostringstream() << "Read from directory " << absolutePaths.back().data()));
    return FAIL; // Return FAIL on system error
}

/* Try to open next directory from list of search roots,
initialize search of the directory properly */
int InputScanner::init()
{
    if (rootDirectories.size() == 0)
        return UNDEFINED;

    DIR *dirStream = NULL;
    std::string path;

    do // Do until some directory is opened successfully
    {
        path = rootDirectories.back().append(DEFAULT_SEPARATOR);
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
        absolutePaths.push_back(std::move(path));
        return OK;
    }
    return UNDEFINED;
}

/* Iterate through file system until some file is opened successfuly
or END-OF-DIRECTORY is reached */
int InputScanner::inputNextFile(std::ifstream &fDescriptor, std::string &pathName)
{
    bool match = false;
    int rc;
    do
    {
        rc = findNextFDRec(fDescriptor, pathName);
        match = std::regex_match(pathName, regex);
        if (!match && fDescriptor.is_open())
            fDescriptor.close();
    } while (rc == FAIL || (!rc && !match));

    while (rc == UNDEFINED)
    {
        if (init() == UNDEFINED)
            break;
        do
        {
            rc = findNextFDRec(fDescriptor, pathName);
            match = std::regex_match(pathName, regex);
            if (!match && fDescriptor.is_open())
                fDescriptor.close();
        } while (rc == FAIL || (!rc && !match));
    }

    return rc;
}

/* Performs test whether the object defined by path is directory
based on platform currently in use */
bool InputScanner::isDirectory(std::string &path)
{
#if defined(__linux__)
    stat(path.data(), &buffer);
    return S_ISDIR(buffer.st_mode) ? true : false;
#elif defined(_WIN32)
    return (GetFileAttributesA(path.data()) == FILE_ATTRIBUTE_DIRECTORY) ? true : false;
#endif
}

/* Print error message in common format along with specific error description */
void InputScanner::printErr(int errNum, const std::ostringstream &errInfo)
{
    printFailed(errInfo);
    std::cerr << " - " << strerror(errNum) << "\n";
}
