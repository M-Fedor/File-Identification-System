#include "InputScanner.h"

InputScanner::InputScanner(std::string rootDirectory)
{
    rootDirectories.push_back(rootDirectory);
}

InputScanner::InputScanner(std::vector<std::string> rootDirectories)
{
    this->rootDirectories = rootDirectories;
}

InputScanner::~InputScanner()
{
    while (directoryStreams.size() != 0)
    {
        if (closedir(directoryStreams.back()) == -1)
        {
            int temp_errno = errno;
            printf("\033[31mFAILED\033[0m to close directory\033[1m %s\033[0m\n",
                   absolutePaths.back().data());
            perror(strerror(temp_errno));
        }
        directoryStreams.pop_back();
        absolutePaths.pop_back();
    }
}

int InputScanner::findNextFDRec(std::string &pathName)
{
    errno = 0;
    struct dirent *dirContent = readdir(directoryStreams.back());

    if (errno == 0)
    {
        if (dirContent == NULL)
        {
            if (closedir(directoryStreams.back()) == -1)
            {
                int temp_errno = errno;
                printf("\033[31mFAILED\033[0m to close directory\033[1m %s\033[0m\n",
                       absolutePaths.back().data());
                perror(strerror(temp_errno));
            }

            directoryStreams.pop_back();
            absolutePaths.pop_back();
            if (directoryStreams.size() > 0)
            {
                int fd = findNextFDRec(pathName);
                return fd;
            }
            else
                return -1;
        }
        else if (dirContent->d_type == DT_DIR)
        {
            if (strcmp(dirContent->d_name, ".") != 0 && strcmp(dirContent->d_name, "..") != 0)
            {
                std::string path = absolutePaths.back();
                DIR *dirStream = opendir(path.append(dirContent->d_name).append("/").data());

                if (dirStream != NULL)
                {
                    printf("\033[32mSUCCESSFUL\033[0m to open directory\033[1m %s\033[0m\n", path.data());
                    directoryStreams.push_back(dirStream);
                    absolutePaths.push_back(path);
                    int fd = findNextFDRec(pathName);
                    return fd;
                }
                else
                {
                    int temp_errno = errno;
                    printf("\033[31mFAILED\033[0m to open directory\033[1m %s\033[0m\n", path.data());
                    perror(strerror(temp_errno));
                }
            }
        }
        else
        {
            std::string path = absolutePaths.back();
            int fd = open(path.append(dirContent->d_name).data(), O_RDONLY);

            if (fd != -1)
            {
                printf("\033[32mSUCCESSFUL\033[0m to open file\033[1m %s\033[0m\n", path.data());
                pathName.assign(path);
                return fd;
            }
            else
            {
                int temp_errno = errno;
                printf("\033[31mFAILED\033[0m to open file\033[1m %s\033[0m\n", path.data());
                perror(strerror(temp_errno));
            }
        }
    }
    else
    {
        int temp_errno = errno;
        printf("\033[31mFAILED\033[0m to read from directory\033[1m %s\033[0m.\n",
               absolutePaths.back().data());
        perror(strerror(temp_errno));
    }

    return -2;
}

int InputScanner::getNextFD(std::string &pathName)
{
    int fd = -3;
    do
        fd = findNextFDRec(pathName);
    while (fd == -2);

    while (fd == -1)
    {
        if (init() == -1)
            break;
        do
            fd = findNextFDRec(pathName);
        while (fd == -2);
    }

    return fd;
}

int InputScanner::init()
{
    if (rootDirectories.size() == 0)
        return -1;

    DIR *dirStream = NULL;
    std::string path;
    do
    {
        path = rootDirectories.back().append("/");
        dirStream = opendir(path.data());

        if (dirStream == NULL)
        {
            int temp_errno = errno;
            printf("\033[31mFAILED\033[0m to open directory\033[1m %s\033[0m\n",
                   rootDirectories.back().data());
            perror(strerror(temp_errno));
        }
        rootDirectories.pop_back();
    } while (dirStream == NULL && rootDirectories.size() != 0);

    if (dirStream != NULL)
    {
        directoryStreams.push_back(dirStream);
        absolutePaths.push_back(path);
        return 0;
    }
    else
        return -1;
}
