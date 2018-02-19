#pragma once

#include <string>
#include <type_traits>
#include <memory>
#include <vector>
#include <Python.h>
#include "Common.h"
#include "CPyModuleContainer.h"
#include "CPythonMetaClass.h"

namespace pycppconn {
    class TypeState;
    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &doc);
        void AddType(std::unique_ptr<TypeState>&& type);
    private:
        std::unique_ptr<PyObject, Deleter::Func> m_module;
        std::vector<std::unique_ptr<TypeState>> m_types;
        std::string m_name;
        std::string m_doc;
    };


#define INIT_MODULE(name, doc) \
    void InitializeModule(CPythonModule& module); \
    PyMODINIT_FUNC init##name() { \
        auto module = std::make_shared<CPythonModule>(#name, doc); \
        CPyModuleContainer::Instance().AddModule(#name, module); \
        CPythonMetaClass::InitType(); \
        InitializeModule(*module); \
    } \
    void InitializeModule(CPythonModule& module)
}

