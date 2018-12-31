#include "SHA2.h"

SHA2::SHA2()
{
    encoder = new CryptoPP::HexEncoder();
    digestor = new CryptoPP::SHA256();
    digest = new unsigned char[digestor->DIGESTSIZE];
}

SHA2::~SHA2()
{
    delete encoder;
    delete digestor;
    delete[] digest;
}

void SHA2::inputData(char *data, size_t dataLength)
{
    digestor->Update((unsigned char *)data, dataLength);
}

void SHA2::inputDataPart(char *data, size_t dataLength)
{
    digestor->Update((unsigned char *)data, dataLength);
}

std::string SHA2::hashData()
{
    digestor->Final(digest);

    std::string output;
    encoder->Attach(new CryptoPP::StringSink(output));
    encoder->Put(digest, digestor->DIGESTSIZE);
    encoder->MessageEnd();
    encoder->Detach();
    return output;
}