#pragma once

#include <string>
#include <memory>
#include <Python.h>

namespace pycppconn {

    struct TypeState
    {
    public:
        TypeState(const std::string &name, const std::string &doc) :
                Name(name), Doc(doc), PyType(nullptr) {}

        ~TypeState() {
            delete[] PyType->tp_methods;
            delete[] PyType->tp_members;
        }

    public:
        std::string Name;
        std::string Doc;
        std::unique_ptr <PyTypeObject> PyType;
    };
}
