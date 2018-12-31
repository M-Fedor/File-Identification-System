#include "Scanner.h"
#include "SHA2.h"

#include <unistd.h>

int main(int argc, char **argv)
{
    size_t bufferSize = 2000;

    std::string path(argv[1]);
    std::string digest;
    char *buffer = new char[bufferSize];
    int fd = -2;

    Scanner *scanner = new Scanner(path);
    HashAlgorithm *hashAlg = new SHA2();

    scanner->init();
    do
    {
        fd = scanner->getNextFD();
        printf("FileDescriptor: %d\n", fd);
        if (fd != -1)
        {
            while (read(fd, buffer, bufferSize) != 0)
            {
                hashAlg->inputDataPart(buffer, bufferSize);
            }
            digest = hashAlg->hashData();
            printf("%s\n", digest.data());
            lseek(fd, 0, SEEK_SET);

            while (read(fd, buffer, bufferSize) != 0)
            {
                hashAlg->inputDataPart(buffer, bufferSize);
            }
            digest = hashAlg->hashData();
            printf("%s\n", digest.data());

            close(fd);
        }
    } while (fd != -1);

    delete[] buffer;
    delete scanner;
    delete hashAlg;

    return 0;
}