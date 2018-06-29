#pragma once

#include <Python.h>

namespace sweetPy {

    class IMemberAccessor {
    public:
        virtual ~IMemberAccessor() {}

        virtual void Set(PyObject *object, PyObject *rhs) = 0;
        virtual PyObject* Get(PyObject *object) = 0;
    };
}
