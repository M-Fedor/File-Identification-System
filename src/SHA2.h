#ifndef SHA2_h
#define SHA2_h

#include "HashAlgorithm.h"

#if defined(__linux__)
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#elif defined(_WIN32)
#include <filters.h>
#include <hex.h>
#include <sha.h>
#endif

/* Class implements file content hashing using SHA256 */
class SHA2 : public HashAlgorithm
{
public:
  SHA2();
  ~SHA2();

  void inputData(char *data, size_t dataLength);
  void inputDataPart(char *data, size_t dataLength);
  std::string hashData();

private:
  CryptoPP::HexEncoder encoder;
  CryptoPP::SHA256 digestor;
  unsigned char *digest;
};

#endif