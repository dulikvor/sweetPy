#pragma once

#include <string>
#include <memory>
#include <vector>
#include <Python.h>
#include "Common.h"

namespace pycppconn {
    class TypeState;
    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &doc);
        void AddType(std::unique_ptr<TypeState>&& type);
        PyObject* GetModule() const;
    private:
        std::unique_ptr<PyObject, Deleter::Func> m_module;
        std::vector<std::unique_ptr<TypeState>> m_types;
        std::string m_name;
        std::string m_doc;
    };
}

