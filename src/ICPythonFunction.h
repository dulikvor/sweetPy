#pragma once

#include <Python.h>
#include <memory>
#include <functional>

namespace sweetPy {

    class CPythonModule;

    class ICPythonFunction
    {
    public:
        union PyFunctionDef
        {
            PyMethodDef MethodDef;
            ternaryfunc Function;
        };

        virtual ~ICPythonFunction(){}
        virtual std::unique_ptr<PyFunctionDef> ToPython() const = 0;
        virtual void AllocateObjectsTypes(CPythonModule& module) const = 0;
    };
}
