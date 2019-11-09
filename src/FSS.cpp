#include "FSS.h"

/* Executes data validation when object configuration is done */
int execute(ParallelExecutor *exec)
{
    if (exec->init())
        return 1;
    if (errFileName.size() != 0)
        exec->setErrFile(errFileName.data());
    if (verbose)
        exec->setVerbose();

    std::cout << "Executing...\n\n";
    exec->validate();
    return 0;
}

/* Instantiate necessary classes using user-defined parameters and prepare them for the execution */
int executeInFileMode()
{
    std::shared_ptr<InputFile> inFile(new InputFile(inputFileName.data(), regexTarget.data()));
    std::vector<std::shared_ptr<Output>> outputList;
    std::shared_ptr<OutputOffline> out(new OutputOffline(outputFileName.data()));

    for (unsigned int i = 0; i < nCores; i++)
        outputList.emplace_back(new OutputDBConnection(
            out.get(), hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL));
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
    else
    {
        for (unsigned int i = 0; i < nCores; i++)
            outputList.emplace_back(new OutputDBConnection(
                out.get(), hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL));
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
        (std::cout << "Name of output file [Validation_results.txt]: ").flush();
        std::getline(std::cin, outputFileName);
        if (outputFileName.size() == 0)
            outputFileName = std::string("Validation_results.txt");

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
    if (rc != 0)
        return rc > 0 ? 0 : 1;

    getInputOpt();
    getOutputOpt();
    std::cout << "\nInitializing...\n";

    if (inputFile)
        return executeInFileMode();
    else
        return executeInScannerMode();
}

/* Print basic usage information */
void printHelp()
{
    std::cout << "File System Scanner \033[1mv 0.1\033[0m\n\n"
              << "\033[1mUsage:\033[0m fss [options]\n"
              << "\033[1mOptions:\033[0m\n"
              << "\t-f\t--file\t\tInput for validation is expected to be obtained from the file"
              << " generated by fss itself in offline mode.\n\t\t\t\t"
              << "If NOT set, then file system is scanned for files of interest by default.\n"
              << "\t-h\t--help\t\tPrints this help.\n"
              << "\t-o\t--offline\tOffline mode; instead of validation against database,"
              << " computed file identifiers are stored in output file for later use.\n\t\t\t\t"
              << "If NOT set, then connection to database is tried to be created in order to validate data.\n"
              << "\t-V\t--verbose\tEnables verbose mode.\n"
              << "\t-v\t--version\tPrints fss version and licence information.\n\n"
              << "All the remaining necessary information is obtained from user during interactive setup.\n"
              << "In case of any problems, please contact <matej.fedor.mf@gmail.com>.\n\n";
}

/* Print version and licence information */
void printVersion()
{
    std::cout << "File System Scanner \033[1mv0.1\033[0m\n"
              << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
              << "This is free software : you are free to change and redistribute it.\n"
              << "There is NO WARRANTY, to the extent permitted by law.\n\n";
}

/* Resolve user-defined options when starting application */
int resolveOptions(int argc, char **args)
{
    const char *short_options = "fhoVv";
    struct option long_options[] = {{"file", 0, NULL, 'f'},
                                    {"help", 0, NULL, 'h'},
                                    {"offline", 0, NULL, 'o'},
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
            return 1;
        case 'o':
            offline = true;
            break;
        case 'V':
            verbose = true;
            break;
        case 'v':
            printVersion();
            return 1;
        default:
            printHelp();
            return -1;
        }
        nextOption = getopt_long(argc, args, short_options, long_options, NULL);
    }
    return 0;
}

/* Ensures secure input of line of characters with ECHOing switched off */
void secureInput(std::string &input)
{
    termios oldOpt;
    tcgetattr(STDIN_FILENO, &oldOpt);
    termios newOpt = oldOpt;
    newOpt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newOpt);

    std::getline(std::cin, input);
    std::cout << "\n";
    tcsetattr(STDIN_FILENO, TCSANOW, &oldOpt);
}