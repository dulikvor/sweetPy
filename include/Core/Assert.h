#pragma once

#include <Python.h>
#include "core/Source.h"
#include "../Types/ObjectPtr.h"
#include "SPException.h"
#include "Deleter.h"

#define CPYTHON_VERIFY(expression, reason) do{ if(!(expression)) throw CPythonException(PyExc_Exception, __CORE_SOURCE, reason); }while(0)
#define CPYTHON_VERIFY_EXC(expression) do{ if(!(expression)){ \
        PyObject *type, *value, *trace; \
        PyErr_Fetch(&type, &value, &trace); \
        sweetPy::ObjectPtr typeGuard(type, &sweetPy::Deleter::Owner), valueGuard(value, &sweetPy::Deleter::Owner), traceGuard(trace, &sweetPy::Deleter::Owner); \
        sweetPy::ObjectPtr reason(PyUnicode_AsASCIIString(value), &sweetPy::Deleter::Owner); \
        PyErr_Clear(); \
        throw sweetPy::CPythonException(PyExc_Exception, __CORE_SOURCE, PyBytes_AsString(reason.get())); \
        }}while(0)
