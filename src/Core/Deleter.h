#pragma once

#include <Python.h>
#include <memory>
#include <functional>
#include "Lock.h"

namespace sweetPy {
    typedef std::unique_ptr<PyObject, std::function<void(PyObject*)>> object_ptr;

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
