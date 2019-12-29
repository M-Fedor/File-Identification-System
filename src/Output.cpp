#include "Output.h"

Output::Output() {}
Output::~Output() {}

int Output::init() { return OK; }
int Output::outputData(std::string & /* data */) { return OK; }
int Output::outputData(
    std::string & /* digest*/, std::string & /* name */) { return OK; }
