#ifndef SHA2_h
#define SHA2_h

#include "HashAlgorithm.h"
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

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