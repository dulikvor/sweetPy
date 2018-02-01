#pragma once

#include <functional>
#include <Python.h>
#include "Lock.h"

namespace pycppconn {
    struct Deleter {
    public:
        typedef std::function<void(PyObject * )> Func;
        static void Borrow(PyObject *) {}
        static void Owner(PyObject *obj) {
            GilLock();
            Py_XDECREF(obj);
        }
    };
}
