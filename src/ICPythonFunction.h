#pragma once

#include <memory>
#include <Python.h>

namespace pycppconn {

    class CPythonModule;

    class ICPythonFunction
    {
    public:
        virtual std::unique_ptr<PyMethodDef> ToPython() const = 0;
        virtual void AllocateObjectsTypes(CPythonModule& module) const = 0;
    };
}
