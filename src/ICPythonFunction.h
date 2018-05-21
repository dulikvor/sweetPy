#pragma once

#include <memory>
#include <Python.h>

namespace pycppconn {

    class ICPythonFunction
    {
    public:
        virtual std::unique_ptr<PyMethodDef> ToPython() const = 0;
    };
}
