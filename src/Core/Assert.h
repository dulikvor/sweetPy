#pragma once

#include <Python.h>
#include "core/Source.h"
#include "Exception.h"
#include "Deleter.h"

namespace sweetPy {

#define CPYTHON_VERIFY(expression, reason) do{ if(!(expression)) throw CPythonException(PyExc_Exception, __CORE_SOURCE, reason); }while(0)
#define CPYTHON_VERIFY_EXC(expression) do{ if(!(expression)){ \
        PyObject *type, *value, *trace; \
        PyErr_Fetch(&type, &value, &trace); \
        object_ptr typeGuard(type, &Deleter::Owner), valueGuard(value, &Deleter::Owner), traceGuard(trace, &Deleter::Owner); \
        object_ptr reason(PyUnicode_AsASCIIString(value), &Deleter::Owner); \
        PyErr_Clear(); \
        throw CPythonException(PyExc_Exception, __CORE_SOURCE, PyBytes_AsString(reason.get())); \
        }}while(0)
}
