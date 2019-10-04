#pragma once

#include <Python.h>
#include <memory>
#include <functional>

namespace sweetPy{
    typedef std::unique_ptr<PyObject, std::function<void(PyObject*)>> ObjectPtr;
}
