#pragma once

#include <Python.h>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include "Core/Deleter.h"

namespace sweetPy {
    
    class CPythonType;
    class CPythonVariable;
    class CPythonFunction;

    class CPythonModule {
    public:
        explicit CPythonModule(const std::string &name, const std::string &doc);
        ~CPythonModule();
        void AddType(CPythonType* type);
        void EraseType(CPythonType* type);
        void AddVariable(std::unique_ptr<CPythonVariable>&& variable);
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
        std::list<CPythonType*> m_types;
        std::vector<std::unique_ptr<PyMethodDef>> m_descriptors;
        std::vector<std::unique_ptr<CPythonVariable>> m_variables;
        std::vector<std::shared_ptr<CPythonFunction>> m_globalFunctions;
        std::vector<std::pair<std::string, object_ptr>> m_enums;
        std::string m_name;
        std::string m_doc;
    };
}

