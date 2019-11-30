#include "Input.h"

Input::Input() {}
Input::~Input() {}

int Input::init() { return OK; }
int Input::inputNextFile(
    std::ifstream & /* fDescriptor */, std::string & /* pathName */) { return UNDEFINED; }
