#pragma once

#include <string>
#include <Python.h>

namespace sweetPy {

    class ICPythonVariable {
    public:
        ICPythonVariable(const std::string &name) : m_name(name) {}

        virtual ~ICPythonVariable() {}
        virtual std::unique_ptr <PyObject, Deleter::Func> ToPython() const = 0;
        const std::string &GetName() const { return m_name; }

    private:
        std::string m_name;
    };
}

