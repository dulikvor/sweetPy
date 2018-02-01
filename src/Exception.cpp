#include "Exception.h"
#include "Lock.h"

namespace pycppconn{

    void CPythonException::Raise() const {
       GilLock lock;
       PyErr_SetString(m_pyError.get(), m_message.c_str());
    }
}
