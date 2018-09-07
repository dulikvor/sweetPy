#pragma once

#include <Python.h>
#include <string>
#include "Deleter.h"

namespace sweetPy{
    class Dictionary
    {
    public:
        Dictionary(PyObject* const object);
        object_ptr GetObject(PyObject* key);
        std::string GetObjectKey(PyObject* const object);
        bool IsObjectExists(PyObject* const object);

    private:
        object_ptr m_dict;
    };
}