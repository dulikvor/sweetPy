#pragma once

#include <Python.h>
#include "Core/src/Exception.h"


namespace pycppconn{

    class CPythonException: public core::Exception
    {
    public:
        template<typename... Args>
        CPythonException(PyObject* pyError, const Source& source, const char* format, Args&&... args):
                core::Exception(source, format, std::forward<Args>(args)...), m_pyError(pyError){
        }
        void Raise() const;

    private:
        std::unique_ptr<PyObject> m_pyError;
    };
}
