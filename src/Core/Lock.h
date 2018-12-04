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

    struct GilRelease
    {
    public:
        GilRelease():m_save(nullptr)
        {
            if(PyGILState_Check() && _PyThreadState_UncheckedGet() != nullptr && m_alreadyReleased == false)
            {
                m_save = PyEval_SaveThread();
                m_alreadyReleased = true;
            }
        }
        ~GilRelease()
        {
            if(m_save)
            {
                PyEval_RestoreThread(m_save);
                m_alreadyReleased = false;
            }
        }
    private:
        thread_local static bool m_alreadyReleased;
        PyThreadState* m_save;
    };
}

