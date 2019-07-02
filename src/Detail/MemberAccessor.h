#pragma once

#include <Python.h>

namespace sweetPy {
    
    class MemberAccessor
    {
    public:
        virtual ~MemberAccessor() = default;
        virtual void set(PyObject *object, PyObject *rhs) = 0;
        virtual PyObject* get(PyObject *object) = 0;
    };
}
