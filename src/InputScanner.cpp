#include "InputScanner.h"

/* Constructor, set root of search as single directory */
InputScanner::InputScanner(std::string &rootDirectory, const char *pattern)
{
    rootDirectories.push_back(rootDirectory);
    regex = pattern ? std::regex(pattern) : std::regex(".*");
}

/* Constructor, set roots of serach as list of directories */
InputScanner::InputScanner(
    std::vector<std::string> &rootDirectories, const char *pattern)
{
    this->rootDirectories.assign(rootDirectories.begin(), rootDirectories.end());
    regex = pattern ? std::regex(pattern) : std::regex(".*");
}

/* Destructor */
InputScanner::~InputScanner()
{
    while (!directoryStreams.empty())
    {
        if (closedir(directoryStreams.back()) == -1)
            printErr(errno, static_cast<std::ostringstream &>(
                                std::ostringstream() << "\033[31mFAILED\033[0m to close directory\033[1m "
                                                     << absolutePaths.back().data() << "\033[0m\n"));
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
                                    std::ostringstream() << "\033[31mFAILED\033[0m to close directory\033[1m"
                                                         << absolutePaths.back().data() << "\033[0m\n"));
            directoryStreams.pop_back();
            absolutePaths.pop_back();
            // If closed directory is subdirectory of another opened one, continue searching that one
            return !directoryStreams.empty() ? findNextFDRec(fDescriptor, pathName) : -1;
        }
        else if (dirContent->d_type == DT_DIR) // Is the next item in directory another directory?
        {
            // Ignore ./ and ../
            if (strcmp(dirContent->d_name, ".") != 0 && strcmp(dirContent->d_name, "..") != 0)
            {
                std::string path = absolutePaths.back(); // Push new path and stream to respective lists ...
                DIR *dirStream = opendir(path.append(dirContent->d_name).append("/").data());
                if (dirStream != NULL)
                {
                    directoryStreams.push_back(dirStream);
                    absolutePaths.push_back(path);
                    return findNextFDRec(fDescriptor, pathName);
                }
                else
                    printErr(errno, static_cast<std::ostringstream &>(
                                        std::ostringstream() << "\033[31mFAILED\033[0m to open directory\033[1m"
                                                             << path.data() << "\033[0m\n"));
            }
        }
        else // Is the next item in directory anything else?
        {
            std::string path = absolutePaths.back();
            fDescriptor.open(path.append(dirContent->d_name).data());
            if (fDescriptor.good())
            {
                pathName.assign(path);
                return 0;
            }
            else
                std::cerr << "\033[31mFAILED\033[0m to open file\033[1m"
                          << path.data() << "\033[0m\n";
        }
    }
    else
        printErr(errno, static_cast<std::ostringstream &>(
                            std::ostringstream() << "\033[31mFAILED\033[0m to read from directory\033[1m"
                                                 << absolutePaths.back().data() << "\033[0m.\n"));
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
                                std::ostringstream() << "\033[31mFAILED\033[0m to open directory\033[1m"
                                                     << rootDirectories.back().data() << "\033[0m\n"));
        rootDirectories.pop_back();
    } while (dirStream == NULL && rootDirectories.size() != 0);

    // Push new streams and paths to corresponding lists, so next time those will be observed
    if (dirStream != NULL)
    {
        directoryStreams.push_back(dirStream);
        absolutePaths.push_back(path);
        return 0;
    }
    else
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
        match = std::regex_match(pathName.data(), regex);
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
            match = std::regex_match(pathName.data(), regex);
            if (!match && fDescriptor.is_open())
                fDescriptor.close();
        } while (rc == -2 || (!rc && !match));
    }

    return rc;
}

/* Print error details */
void InputScanner::printErr(int errNum, const std::ostringstream &errInfo)
{
    std::cerr << errInfo.str().data();
    std::cerr << strerror(errNum);
}
