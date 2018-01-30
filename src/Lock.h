#pragma once

#include <Python.h>

namespace pycppconn {

    struct GilLock
    {
    public:
        GilLock() { m_state = PyGILState_Ensure(); }
        ~GilLock() { PyGILState_Release(m_state); }
    private:
        PyGILState_STATE m_state;
    };
}

