#include "Scanner.h"

Scanner::Scanner(std::string rootDirectory)
{
    rootDirectories.push_back(rootDirectory);
}

Scanner::Scanner(std::vector<std::string> rootDirectories)
{
    this->rootDirectories = rootDirectories;
}

Scanner::~Scanner()
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

int Scanner::findNextFDRec()
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
                int fd = findNextFDRec();
                return fd;
            }
            else
                return -1;
        }
        else if (dirContent->d_type == DT_DIR)
        {
            std::string path = absolutePaths.back().append(dirContent->d_name);
            DIR *dirStream = opendir(path.data());

            if (dirStream != NULL)
            {
                directoryStreams.push_back(dirStream);
                absolutePaths.push_back(path);
                int fd = findNextFDRec();
                return fd;
            }
            else
            {
                int temp_errno = errno;
                printf("\033[31mFAILED\033[0m to open directory\033[1m %s\033[0m\n", path.data());
                perror(strerror(temp_errno));
            }
        }
        else
        {
            std::string path = absolutePaths.back().append(dirContent->d_name);
            int fd = open(path.data(), O_RDONLY);

            if (fd != -1)
                return fd;
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

int Scanner::getNextFD()
{
    int fd = -3;
    do
        fd = findNextFDRec();
    while (fd == -2);

    while (fd == -1)
    {
        if (init() == -1)
            break;
        do
            fd = findNextFDRec();
        while (fd == -2);
    }

    return fd;
}

int Scanner::init()
{
    if (rootDirectories.size() == 0)
        return -1;

    DIR *dirStream = NULL;
    do
    {
        dirStream = opendir(rootDirectories.back().data());

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
        absolutePaths.push_back(rootDirectories.back());
        return 0;
    }
    else
        return -1;
}
