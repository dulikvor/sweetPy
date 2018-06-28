#pragma once

#include <Python.h>

namespace pycppconn {

    class IMemberAccessor {
    public:
        virtual ~IMemberAccessor() {}

        virtual void Set(PyObject *object, PyObject *rhs) = 0;
    };
}
