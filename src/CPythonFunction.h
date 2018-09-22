#pragma once

#include <Python.h>
#include <memory>
#include <string>
#include "CPyModuleContainer.h"

namespace sweetPy {

    class CPythonModule;

    class CPythonFunction
    {
    public:
        CPythonFunction(const std::string& name, const std::string& doc);
        virtual ~CPythonFunction();
        CPythonFunction(CPythonFunction &) = delete;
        CPythonFunction &operator=(CPythonFunction &) = delete;
        CPythonFunction(CPythonFunction &&obj);
        CPythonFunction &operator=(CPythonFunction &&obj);

        virtual std::unique_ptr<PyMethodDef> ToPython() const = 0;
        virtual void AllocateTypes(CPythonModule& module) const = 0;

        template<typename CPythonFunctionType>
        static int GenerateFunctionId(const std::string& functionName)
        {
            size_t nameHash = std::hash<std::string>{}(functionName);
            size_t typeHash = typeid(CPythonFunctionType).hash_code();
            return nameHash ^ (typeHash << 1);
        }

    protected:
        std::string m_name;
        std::string m_doc;
    };
}
