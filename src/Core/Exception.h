#pragma once

#include <Python.h>
#include <memory>
#include "core/Exception.h"
#include "Deleter.h"


namespace sweetPy{

    class CPythonException: public core::Exception
    {
    public:
        template<typename... Args>
        CPythonException(PyObject* pyError, const core::Source& source, const char* format, Args&&... args):
                core::Exception(source, format, std::forward<Args>(args)...), m_pyError(pyError, &Deleter::Borrow){
        }
        void Raise() const;

    private:
        object_ptr m_pyError;
    };
}
