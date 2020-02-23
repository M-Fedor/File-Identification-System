#include "InputScanner.h"

/* Constructor, set root of search as single directory */
InputScanner::InputScanner(
    std::string &rootDirectory, const char *pattern) : hasNextAlternateStream(false)
{
    rootDirectories.push_back(std::move(rootDirectory));
    regex = std::regex(pattern);
}

/* Constructor, set roots of serach as list of directories */
InputScanner::InputScanner(
    std::vector<std::string> &rootDirectories, const char *pattern) : hasNextAlternateStream(false)
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

int InputScanner::enumerateNextAlternateStream(std::ifstream &fDescriptor, std::string &pathName)
{
#if defined(__linux__)
    pathName = currentPathName;
    pathName.append(":").append(currentPosition);
    size_t streamNameSize = std::strlen(currentPosition) + 1;
    currentPosition += streamNameSize;
    attrSize -= streamNameSize;
    hasNextAlternateStream = (attrSize > 0) ? true : false;
#elif defined(_WIN32)
    std::wstring pathStreamNameW(currentPathNameW);
    pathStreamNameW.append(streamData.cStreamName);
    pathName = UTF16ToUTF8(pathStreamNameW);
    hasNextAlternateStream = getNextAlternateStream(pathName);
#endif

    fDescriptor.open(pathName);
    if (fDescriptor.fail())
        return printFailed(static_cast<std::ostringstream &>(
            std::ostringstream() << "Open file " << pathName.data()));
    return OK;
}

/* Iterate through file-system, return next file's opened file descriptor for reading,
fill absolute path of the file in pathName, return UNDEFINED when no more files can be found
in current root of search */
int InputScanner::findNextFDRec(std::ifstream &fDescriptor, std::string &pathName)
{
    errno = 0; // readdir returns NULL and doesn't change errno on END-OF-DIRECTORY
    struct dirent *dirContent = readdir(directoryStreams.back());

    if (errno) // Did error occur?
        return printErr(errno, static_cast<std::ostringstream &>(
                                   std::ostringstream() << "Read from directory " << absolutePaths.back().data()));

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
        if (!std::strcmp(dirContent->d_name, ".") || !std::strcmp(dirContent->d_name, ".."))
            return FAIL;

        DIR *dirStream = opendir(path.append(DEFAULT_SEPARATOR).data());
        if (dirStream == NULL)
            return printErr(errno, static_cast<std::ostringstream &>(
                                       std::ostringstream() << "Open directory " << path.data()));
        directoryStreams.push_back(dirStream);
        absolutePaths.push_back(std::move(path));
        if (hasAlternateStreamDir(absolutePaths.back()))
            return enumerateNextAlternateStream(fDescriptor, pathName);
        return findNextFDRec(fDescriptor, pathName);
    }
    else // Is the next item in directory anything else?
    {
        fDescriptor.open(path.data());
        if (fDescriptor.fail())
            return printFailed(static_cast<std::ostringstream &>(
                std::ostringstream() << "Open file " << path.data()));
        pathName = std::move(path);
        hasAlternateStreamFile(pathName);
        return OK;
    }
}

#if defined(_WIN32)
bool InputScanner::getFirstAlternateStream(std::string &pathName)
{
    nextAlternateStream = FindFirstStreamW(
        currentPathNameW.data(), FindStreamInfoStandard, (LPVOID *)&streamData, 0);
    if (nextAlternateStream == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err != ERROR_HANDLE_EOF)
            printFailed(static_cast<std::ostringstream &>(
                std::ostringstream() << "Obtain Alternate Data Stream for object "
                                     << pathName << " (code " << err << ")"));
        return false;
    }
    return true;
}

bool InputScanner::getNextAlternateStream(std::string &pathName)
{
    if (!FindNextStreamW(nextAlternateStream, (LPVOID *)&streamData))
    {
        DWORD err = GetLastError();
        if (err != ERROR_HANDLE_EOF)
            printFailed(static_cast<std::ostringstream &>(
                std::ostringstream() << "Obtain Alternate Data Stream for object "
                                     << pathName << " (code " << err << ")"));
        FindClose(nextAlternateStream);
        return false;
    }
    return true;
}
#endif

bool InputScanner::hasAlternateStreamDir(std::string &pathName)
{
#if defined(__linux__)
    return hasAlternateStreamFile(pathName);
#elif defined(_WIN32)
    currentPathNameW = MultiByteToUTF16(pathName);
    hasNextAlternateStream = getFirstAlternateStream(pathName);
    return hasNextAlternateStream;
#endif
}

bool InputScanner::hasAlternateStreamFile(std::string &pathName)
{
#if defined(__linux__)
    attrSize = getxattr(pathName.data(), "ntfs.streams.list", NULL, 0);
    hasNextAlternateStream = false;
    if (attrSize == -1)
        printErr(errno, static_cast<std::ostringstream &>(
                            std::ostringstream() << "Obtain list of Alternate Data Streams for object " << pathName));
    if (attrSize <= 0)
        return false;

    attr.reset(new char[attrSize + 1]);
    if (getxattr(pathName.data(), "ntfs.streams.list", attr.get(), attrSize) == -1)
    {
        printErr(errno, static_cast<std::ostringstream &>(
                            std::ostringstream() << "Obtain list of Alternate Data Streams for object " << pathName));
        return false;
    }
    currentPathName = pathName;
    currentPosition = attr.get();
    hasNextAlternateStream = true;
    attr.get()[attrSize] = 0;
    return true;
#elif defined(_WIN32)
    currentPathNameW = MultiByteToUTF16(pathName);
    if (getFirstAlternateStream(pathName))
        hasNextAlternateStream = getNextAlternateStream(pathName);
    return hasNextAlternateStream;
#endif
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
    int rc = loadFile(fDescriptor, pathName);
    while (rc == UNDEFINED)
    {
        if (init() == UNDEFINED)
            break;
        rc = loadFile(fDescriptor, pathName);
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

int InputScanner::loadFile(std::ifstream &fDescriptor, std::string &pathName)
{
    bool match = false;
    int rc;
    do
    {
        if (hasNextAlternateStream)
            rc = enumerateNextAlternateStream(fDescriptor, pathName);
        else
            rc = findNextFDRec(fDescriptor, pathName);
        match = std::regex_match(pathName, regex);
        if (!match && fDescriptor.is_open())
            fDescriptor.close();
    } while (rc == FAIL || (!rc && !match));

    return rc;
}

/* Print error message in common format along with specific error description */
int InputScanner::printErr(int errNum, const std::ostringstream &errInfo)
{
    printFailed(errInfo);
    std::cerr << " - " << strerror(errNum) << "\n";
    return FAIL;
}
