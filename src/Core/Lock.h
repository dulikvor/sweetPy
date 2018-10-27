#pragma once

#include <Python.h>

namespace sweetPy {

    struct GilLock
    {
    public:
        GilLock() { m_state = PyGILState_Ensure(); }
        ~GilLock() { PyGILState_Release(m_state); }
    private:
        PyGILState_STATE m_state;
    };

    struct Yield
    {
    public:
        Yield():m_save(nullptr)
        {
            if(m_alreadyYield == false && PyGILState_Check())
            {
                m_save = PyEval_SaveThread();
                m_alreadyYield = true;
            }
        }
        ~Yield()
        {
            if(m_save)
            {
                PyEval_RestoreThread(m_save);
                m_alreadyYield = false;
            }
        }
    private:
        thread_local static bool m_alreadyYield;
        PyThreadState* m_save;
    };
}

