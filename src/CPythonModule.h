#pragma once

#include <string>
#include <type_traits>
#include <memory>
#include <Python.h>
#include "Common.h"

namespace pycppconn {
    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &docString);

    private:
        std::unique_ptr<PyObject, Deleter::Func> m_module;
    };


#define INIT_MODULE(name, doc) \
    void InitializeModule(CPythonModule& module); \
    PyMODINIT_FUNC init##name() { \
        static_assert(std::is_same<typeof(doc), const char*>::value); \
        CPythonModule module("name", doc); \
        InitializeModule(module); \
    } \
    void InitializeModule(CPythonModule& module)
}

