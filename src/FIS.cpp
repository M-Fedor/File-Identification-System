#include "FIS.h"

/* Executes data validation when object configuration is done */
int execute(ParallelExecutor *exec)
{
    if (exec->init())
        return FAIL;
    if (errFileName.size() != 0)
        exec->setErrFile(errFileName.data());
    if (verbose)
        exec->setVerbose();

    std::cout << "Executing...\n\n";
    exec->execute();
    return OK;
}

/* Instantiate necessary classes using user-defined parameters and prepare them for the execution */
int executeInFileMode()
{
    std::shared_ptr<InputFile> inFile(new InputFile(inputFileName.data(), regexTarget.data()));
    std::vector<std::shared_ptr<Output>> outputList;
    std::shared_ptr<OutputOffline> out(new OutputOffline(outputFileName.data()));

#if defined(_WIN32)
    if (update)
    {
        for (unsigned int i = 0; i < nCores; i++)
        {
            DBConnection conn(hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL);
            outputList.emplace_back(new OutputUpdateDB(conn));
        }
    }
#endif

    if (!offline)
    {
        for (unsigned int i = 0; i < nCores; i++)
        {
            DBConnection conn(hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL);
            outputList.emplace_back(new OutputValidateDB(conn, out));
        }
    }
    std::shared_ptr<ParallelExecutor> exec = std::make_shared<ParallelExecutor>(inFile, outputList);

    return execute(exec.get());
}

/* Instantiate necessary classes using user-defined parameters and prepare them for the execution */
int executeInScannerMode()
{
    std::shared_ptr<InputScanner> inScanner(new InputScanner(rootDirectories, regexTarget.data()));
    std::vector<std::shared_ptr<HashAlgorithm>> hashAlgList;
    std::vector<std::shared_ptr<Output>> outputList;
    std::shared_ptr<OutputOffline> out(new OutputOffline(outputFileName.data()));
    std::shared_ptr<ParallelExecutor> exec;

    for (unsigned int i = 0; i < nCores; i++)
        hashAlgList.emplace_back(new SHA2());
    if (offline)
        exec = std::make_shared<ParallelExecutor>(inScanner, hashAlgList, out);

#if defined(_WIN32)
    if (update)
    {
        for (unsigned int i = 0; i < nCores; i++)
        {
            DBConnection conn(hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL);
            outputList.emplace_back(new OutputUpdateDB(conn));
        }
        exec = std::make_shared<ParallelExecutor>(inScanner, hashAlgList, outputList);
    }
#endif

    if (!offline && !update)
    {
        for (unsigned int i = 0; i < nCores; i++)
        {
            DBConnection conn(hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL);
            outputList.emplace_back(new OutputValidateDB(conn, out));
        }
        exec = std::make_shared<ParallelExecutor>(inScanner, hashAlgList, outputList);
    }

    return execute(exec.get());
}

/* Prompt user for input component configuration */
void getInputOpt()
{
    if (inputFile)
    {
        offline = false;
        do
        {
            (std::cout << "Path to input file []: ").flush();
            std::getline(std::cin, inputFileName);
        } while (inputFileName.size() == 0);
    }
    else
    {
        std::string rootDir;
        do
        {
            do
            {
                (std::cout << "Root-of-search directory []: ").flush();
                std::getline(std::cin, rootDir);
            } while (rootDir.size() == 0);
            rootDirectories.push_back(std::string(rootDir));
            do
            {
                (std::cout << "Want to specify another directory? [y/n]: ").flush();
                std::getline(std::cin, rootDir);
            } while (rootDir.size() == 0);
        } while (!strcmp(rootDir.data(), "Y") || !strcmp(rootDir.data(), "y") ||
                 !strcmp(rootDir.data(), "yes"));
    }
    (std::cout << "Regular expression for input filtering [.*]: ").flush();
    std::getline(std::cin, regexTarget);
    if (regexTarget.size() == 0)
        regexTarget = std::string(".*");
    std::cout << "\n";
}

/* Prompt user for output component configuration */
void getOutputOpt()
{
    std::string value;
    if (offline)
    {
        (std::cout << "Name of output file [Offline_scan.txt]: ").flush();
        std::getline(std::cin, outputFileName);
        if (outputFileName.size() == 0)
            outputFileName = std::string("Offline_scan.txt");
    }
    else
    {
        (std::cout << "Database hostname [localhost]: ").flush();
        std::getline(std::cin, hostName);
        if (hostName.size() == 0)
            hostName = std::string("localhost");

        (std::cout << "Database port [3306]: ").flush();
        std::getline(std::cin, value);
        int rc = std::atoi(value.data());
        dbPort = (value.size() == 0 || rc <= 0) ? 3306 : rc;

        (std::cout << "Database name [test]: ").flush();
        std::getline(std::cin, dbName);
        if (dbName.size() == 0)
            dbName = std::string("test");

        (std::cout << "Database username [root]: ").flush();
        std::getline(std::cin, userName);
        if (userName.size() == 0)
            userName = std::string("root");

        (std::cout << "Database password: ").flush();
        secureInput(password);
    }
    if (!offline && !update)
    {
        (std::cout << "Name of output file [Validation_results.txt]: ").flush();
        std::getline(std::cin, outputFileName);
        if (outputFileName.size() == 0)
            outputFileName = std::string("Validation_results.txt");
    }

    (std::cout << "Name of error-output file []: ").flush();
    std::getline(std::cin, errFileName);

    (std::cout << "Number of jobs run in parallel ["
               << 2 * std::thread::hardware_concurrency() << "]: ")
        .flush();
    std::getline(std::cin, value);
    int rc = std::atoi(value.data());
    nCores = (value.size() == 0 || rc <= 0) ? 2 * std::thread::hardware_concurrency() : rc;
    std::cout << "\n";
}

int main(int argc, char **args)
{
    int rc = resolveOptions(argc, args);
    if (rc != OK)
        return (rc == END) ? OK : rc;

    getInputOpt();
    getOutputOpt();
    std::cout << "\nInitializing...\n";

    return inputFile ? executeInFileMode() : executeInScannerMode();
}

/* Print basic usage information */
void printHelp()
{
    std::cout << "\nFile Identification System v 0.1\n\n"
              << "Usage: fis [options]\n"
              << "Options:\n"
              << "\t-f\t--file\t\tInput for validation is expected to be obtained from the file"
              << " generated by fis itself in offline mode.\n\t\t\t\t"
              << "If NOT set, then file system is scanned for files of interest by default.\n"
              << "\t-h\t--help\t\tPrints this help.\n"
              << "\t-o\t--offline\tOffline mode; instead of validation against database,"
              << " computed file identifiers are stored in output file for later use.\n\t\t\t\t"
              << "If NOT set, then connection to database is tried to be created in order to validate data.\n"
              << "\t-u\t--update\tUpdate mode; provides user with means for feeding extracted"
              << " file metadata into database."
              << "If NOT set, then connection to database is tried to be created in order to validate data.\n"
              << "\t-V\t--verbose\tEnables verbose mode.\n"
              << "\t-v\t--version\tPrints fis version and licence information.\n\n"
              << "All the remaining necessary information is obtained from user during interactive setup.\n"
              << "In case of any problems, please contact <matej.fedor.mf@gmail.com>.\n\n";
}

/* Print version and licence information */
void printVersion()
{
    std::cout << "\nFile Identification System v0.1\n\n"
              << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
              << "This is free software : you are free to change and redistribute it.\n"
              << "There is NO WARRANTY, to the extent permitted by law.\n\n";
}

/* Resolve user-defined options when starting application on UNIX platforms */
#if defined(__linux__)
int resolveOptions(int argc, char **args)
{
    const char *short_options = "fhouVv";
    struct option long_options[] = {{"file", 0, NULL, 'f'},
                                    {"help", 0, NULL, 'h'},
                                    {"offline", 0, NULL, 'o'},
                                    {"update", 0, NULL, 'u'},
                                    {"verbose", 0, NULL, 'V'},
                                    {"version", 0, NULL, 'v'},
                                    {NULL, 0, NULL, 0}};

    char nextOption = getopt_long(argc, args, short_options, long_options, NULL);
    while (nextOption != -1)
    {
        switch (nextOption)
        {
        case 'f':
            inputFile = true;
            break;
        case 'h':
            printHelp();
            return END;
        case 'o':
            offline = true;
            break;
        case 'u':
            std::cout << "This feature is not supported on your OS.\n";
            return END;
        case 'V':
            verbose = true;
            break;
        case 'v':
            printVersion();
            return END;
        default:
            printHelp();
            return FAIL;
        }
        nextOption = getopt_long(argc, args, short_options, long_options, NULL);
    }
    return OK;
}
/* Resolve user-defined options when starting application on Microsoft Windows platforms */
#elif defined(_WIN32)
int resolveOptions(int argc, char **args)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp("-f", args[i]) || !strcmp("--file", args[i]))
            inputFile = true;
        else if (!strcmp("-h", args[i]) || !strcmp("--help", args[i]))
        {
            printHelp();
            return END;
        }
        else if (!strcmp("-o", args[i]) || !strcmp("--offline", args[i]))
            offline = true;
        else if (!strcmp("-u", args[i]) || !strcmp("--update", args[i]))
            update = true;
        else if (!strcmp("-V", args[i]) || !strcmp("--verbose", args[i]))
            verbose = true;
        else if (!strcmp("-v", args[i]) || !strcmp("--version", args[i]))
        {
            printVersion();
            return END;
        }
        else
        {
            printHelp();
            return FAIL;
        }
    }

    return OK;
}
#endif

/* Ensures secure input of line of characters with ECHOing switched off */
void secureInput(std::string &input)
{
#if defined(__linux__)
    termios oldOpt;
    tcgetattr(STDIN_FILENO, &oldOpt);
    termios newOpt = oldOpt;
    newOpt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newOpt);

#elif defined(_WIN32)
    DWORD mode = 0;
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdIn, &mode);
    SetConsoleMode(hStdIn, mode & (~ENABLE_ECHO_INPUT));
#endif

    std::getline(std::cin, input);
    std::cout << "\n";

#if defined(__linux__)
    tcsetattr(STDIN_FILENO, TCSANOW, &oldOpt);
#elif defined(_WIN32)
    SetConsoleMode(hStdIn, mode);
#endif
}
