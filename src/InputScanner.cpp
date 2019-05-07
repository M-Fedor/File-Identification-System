#include "InputScanner.h"

/* Constructor, set root of search as single directory */
InputScanner::InputScanner(std::string &rootDirectory)
{
    rootDirectories.push_back(rootDirectory);
}

/* Constructor, set roots of serach as list of directories */
InputScanner::InputScanner(std::vector<std::string> &rootDirectories)
{
    this->rootDirectories.assign(rootDirectories.begin(), rootDirectories.end());
}

/* Destructor */
InputScanner::~InputScanner()
{
    while (!directoryStreams.empty())
    {
        if (closedir(directoryStreams.back()) == -1)
        {
            int temp_errno = errno;
            std::cerr << "\033[31mFAILED\033[0m to close directory\033[1m "
                      << absolutePaths.back().data() << "\033[0m\n";
            std::cerr << strerror(temp_errno);
        }
        directoryStreams.pop_back();
        absolutePaths.pop_back();
    }
}

/* Iterate through file-system, return next file's opened file descriptor for reading,
fill absolute path of the file in pathName, return -1 when no more files can be found
in current root of search */
int InputScanner::findNextFDRec(std::ifstream *fDescriptor, std::string &pathName)
{
    // readdir returns NULL and doesn't change errno on END-OF-DIRECTORY
    errno = 0;
    struct dirent *dirContent = readdir(directoryStreams.back());

    // Did error occur?
    if (errno == 0)
    {
        // Was the END-OF-DIRECTORY reached?
        if (dirContent == NULL)
        {
            if (closedir(directoryStreams.back()) == -1)
            {
                int temp_errno = errno;
                std::cerr << "\033[31mFAILED\033[0m to close directory\033[1m"
                          << absolutePaths.back().data() << "\033[0m\n";
                std::cerr << strerror(temp_errno);
            }

            directoryStreams.pop_back();
            absolutePaths.pop_back();

            // If closed directory is subdirectory of another opened one, continue searching that one
            if (!directoryStreams.empty())
                return findNextFDRec(fDescriptor, pathName);
            else
                // No more files in current root of search
                return -1;
        }
        // Is the next item in directory another directory?
        else if (dirContent->d_type == DT_DIR)
        {
            // Ignore ./ and ../
            if (strcmp(dirContent->d_name, ".") != 0 && strcmp(dirContent->d_name, "..") != 0)
            {
                // Push new path and stream to respective lists ...
                std::string path = absolutePaths.back();
                DIR *dirStream = opendir(path.append(dirContent->d_name).append("/").data());

                if (dirStream != NULL)
                {
                    std::cout << "\033[32mSUCCESSFUL\033[0m to open directory\033[1m "
                              << path.data() << "\033[0m\n";
                    directoryStreams.push_back(dirStream);
                    absolutePaths.push_back(path);
                    // ... And continue searching in the subdirectory
                    return findNextFDRec(fDescriptor, pathName);
                }
                else
                {
                    int temp_errno = errno;
                    std::cerr << "\033[31mFAILED\033[0m to open directory\033[1m"
                              << path.data() << "\033[0m\n";
                    std::cerr << strerror(temp_errno);
                }
            }
        }
        // Is the next item in directory anything else?
        else
        {
            // Then open it and let's observe ...
            std::string path = absolutePaths.back();
            fDescriptor->open(path.append(dirContent->d_name).data());

            if (fDescriptor->good())
            {
                std::cout << "\033[32mSUCCESSFUL\033[0m to open file\033[1m"
                          << path.data() << "\033[0m\n";
                pathName.assign(path);
                return 0;
            }
            else
                std::cerr << "\033[31mFAILED\033[0m to open file\033[1m"
                          << path.data() << "\033[0m\n";
        }
    }
    else
    {
        int temp_errno = errno;
        std::cerr << "\033[31mFAILED\033[0m to read from directory\033[1m"
                  << absolutePaths.back().data() << "\033[0m.\n";
        std::cerr << strerror(temp_errno);
    }

    // Return -2 on system error
    return -2;
}

/* Try to open next directory from list of search roots,
initialize search of this directory properly */
int InputScanner::init()
{
    if (rootDirectories.size() == 0)
        return -1;

    DIR *dirStream = NULL;
    std::string path;

    // Do until some directory is opened successfully
    do
    {
        path = rootDirectories.back().append("/");
        dirStream = opendir(path.data());

        if (dirStream == NULL)
        {
            int temp_errno = errno;
            std::cerr << "\033[31mFAILED\033[0m to open directory\033[1m"
                      << rootDirectories.back().data() << "\033[0m\n";
            std::cerr << strerror(temp_errno);
        }
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
int InputScanner::inputNextFile(std::ifstream *fDescriptor, std::string &pathName)
{
    int rc = -3;
    do
        rc = findNextFDRec(fDescriptor, pathName);
    while (rc == -2);

    while (rc == -1)
    {
        if (init() == -1)
            break;
        do
            rc = findNextFDRec(fDescriptor, pathName);
        while (rc == -2);
    }

    return rc;
}
