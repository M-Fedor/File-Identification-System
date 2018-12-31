#ifndef HashAlgorithm_h
#define HashAlgorithm_h

#include <stdio.h>
#include <string>

class HashAlgorithm
{
  public:
    HashAlgorithm();
    ~HashAlgorithm();

    virtual void inputData(char *data, size_t dataLength);
    virtual void inputDataPart(char *data, size_t dataLength);
    virtual std::string hashData();
};

#endif