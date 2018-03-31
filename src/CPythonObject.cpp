#include "CPythonObject.h"

namespace pycppconn {
    thread_local std::string Object<std::string>::m_value = "";
}
