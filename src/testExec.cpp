#include "Scanner.h"

#include <unistd.h>

int main(int argc, char **argv)
{
    std::string path(argv[1]);
    Scanner *scanner = new Scanner(path);
    int fd = -2;

    scanner->init();
    do
    {
        fd = scanner->getNextFD();
        printf("FileDescriptor: %d\n", fd);
        close(fd);
    } while (fd != -1);

    delete scanner;

    return 0;
}