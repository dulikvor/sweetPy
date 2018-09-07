#include "Exception.h"
#include "src/Core/Lock.h"

namespace sweetPy{

    void CPythonException::Raise() const {
        if(!PyErr_Occurred())
            PyErr_SetString(m_pyError.get(), m_message.c_str());
    }
}
