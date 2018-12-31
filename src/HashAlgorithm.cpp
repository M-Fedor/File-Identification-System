#include "HashAlgorithm.h"

HashAlgorithm::HashAlgorithm() {}
HashAlgorithm::~HashAlgorithm() {}

void HashAlgorithm::inputData(char *data, size_t dataLength)
{
    printf("HashAlgorithm::inputData()");
}

void HashAlgorithm::inputDataPart(char *data, size_t dataLength)
{
    printf("HashAlgorithm::inputDataPart()");
}

std::string HashAlgorithm::hashData()
{
    printf("HashAlgorithm::hashData()");
    return std::string();
}