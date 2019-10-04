#pragma once

#include <Python.h>
#include <memory>
#include <functional>
#include "Lock.h"

namespace sweetPy {
    struct Deleter {
    public:
        static void Borrow(PyObject *) {}
        static void Owner(PyObject *obj) {
            GilLock();
            Py_XDECREF(obj);
        }
    };
}
