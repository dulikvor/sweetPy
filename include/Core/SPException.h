#pragma once

#include <Python.h>
#include "core/Exception.h"
#include "../Types/ObjectPtr.h"
#include "Deleter.h"

namespace sweetPy{

    class CPythonException: public core::Exception
    {
    public:
        template<typename... Args>
        CPythonException(PyObject* pyError, const core::Source& source, const char* format, Args&&... args):
                core::Exception(source, format, std::forward<Args>(args)...), m_pyError(pyError, &Deleter::Borrow){}
        void raise() const
        {
            if(!PyErr_Occurred())
                PyErr_SetString(m_pyError.get(), m_message.c_str());
        }

    private:
        ObjectPtr m_pyError;
    };
}
