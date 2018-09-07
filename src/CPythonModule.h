#pragma once

#include <Python.h>
#include <string>
#include <memory>
#include <vector>
#include "src/Core/Deleter.h"
#include "ICPythonVariable.h"
#include "CPythonFunction.h"
#include "CPythonType.h"

namespace sweetPy {

    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &doc);
        ~CPythonModule();
        void AddType(CPythonType* type);
        void AddVariable(std::unique_ptr<ICPythonVariable>&& variable);
        void AddGlobalFunction(const std::shared_ptr<CPythonFunction>& function);
        void AddEnum(const std::string& name, object_ptr&& dictionary);
        PyObject* GetModule() const;
        void Finalize();

    private:
        void InitGlobalFunctions();
        void InitVariables();
        void InitTypes();
        void InitEnums();

    private:
        PyModuleDef m_moduleDef;
        std::unique_ptr<PyObject, Deleter::Func> m_module;
        std::vector<CPythonType*> m_types;
        std::vector<std::unique_ptr<PyMethodDef>> m_descriptors;
        std::vector<std::unique_ptr<ICPythonVariable>> m_variables;
        std::vector<std::shared_ptr<CPythonFunction>> m_globalFunctions;
        std::vector<std::pair<std::string, object_ptr>> m_enums;
        std::string m_name;
        std::string m_doc;
    };
}

