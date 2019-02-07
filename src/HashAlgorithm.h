#ifndef HashAlgorithm_h
#define HashAlgorithm_h

#include <string>

/* Interface for file identification component */
class HashAlgorithm
{
public:
  HashAlgorithm();
  virtual ~HashAlgorithm();

  /* Input overall data to be processed */
  virtual void inputData(char *data, size_t dataLength);

  /* Input (repetitively) partial data to be processed */
  virtual void inputDataPart(char *data, size_t dataLength);

  /* Compute unique identifier for input data */
  virtual std::string hashData();
};

#endif