#pragma once

#include <string>
#include <memory>
#include <vector>
#include <Python.h>
#include "Common.h"

namespace sweetPy {
    class CPythonType;
    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &doc);
        void AddType(std::unique_ptr<CPythonType>&& type);
        PyObject* GetModule() const;
    private:
        std::unique_ptr<PyObject, Deleter::Func> m_module;
        std::vector<std::unique_ptr<CPythonType>> m_types;
        std::string m_name;
        std::string m_doc;
    };
}

