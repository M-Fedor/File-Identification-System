#include "SysUpdate.h"

#if defined(_WIN32)

/* Execute specified operation using user-defined options. */
int execute()
{
    SysUpdateImp sysUpdate(verbose);

    std::cout << "Initializing...\n";
    if (sysUpdate.init())
        return FAIL;

    std::cout << "Executing...\n";
    if (list)
        sysUpdate.list();
    else if (update)
    {
        int rc = sysUpdate.update();
        if (rc)
            return rc;
    }
    return OK;
}

int main(int argc, char **args)
{
    int rc = resolveOptions(argc, args);
    if (rc)
        return (rc == END) ? END : rc;

    return execute();
}

/* Resolve user-defined options when starting application. */
int resolveOptions(int argc, char **args)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp("-h", args[i]) || !strcmp("--help", args[i]))
        {
            printHelp();
            return END;
        }
        else if (!strcmp("-l", args[i]) || !strcmp("--list", args[i]))
        {
            list = true;
            update = false;
        }
        else if (!strcmp("-u", args[i]) || !strcmp("--update", args[i]))
        {
            update = true;
            list = false;
        }
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

/* Print basic usage information. */
void printHelp()
{
    std::cout << "\nSystem Update Utility Application v0.1\n\n"
              << "Usage: SysUpdate.exe [options]\n"
              << "Options:\n"
              << "\t-h\t--help\t\tPrints this help.\n"
              << "\t-l\t--list\t\tPrints details for all the available updates.\n"
              << "\t-u\t--update\tInstalls all currently available OS updates.\n"
              << "\t-V\t--verbose\tEnables verbose mode, prints details for all the available updates before installation.\n"
              << "\t-v\t--version\tPrints SysUpdate.exe version and licence information.\n\n"
              << "In case of any problems, please contact <matej.fedor.mf@gmail.com>.\n\n";
}

/* Print version and licence information. */
void printVersion()
{
    std::cout << "\nSystem Update Utility Application v0.1\n\n"
              << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
              << "This is free software : you are free to change and redistribute it.\n"
              << "There is NO WARRANTY, to the extent permitted by law.\n\n";
}
#endif
