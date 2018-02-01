#include "CPythonFunction.h"

namespace pycppconn{
    thread_local int ArgumentConvertor<int>::Data = 0;
    thread_local const char* ArgumentConvertor<std::string>::Data = nullptr;
}
