#pragma once

#include <string>
#include <Python.h>
#include "Deleter.h"
#include "ICPythonVariable.h"
#include "CPythonObject.h"

namespace sweetPy {

    //Since a copy is due to happen we only want to support pod types to reduce overhead (huge pod structs are unlikeable)
    template<typename T, typename std::enable_if<std::is_pod<T>::value>::type>
    class CPythonVariable : public ICPythonVariable {
    public:
        CPythonVariable(const std::string &name, const T &value)
                : ICPythonVariable(name), m_value(value) {}

        std::unique_ptr <PyObject, Deleter::Func> ToPython() const override {
            return std::unique_ptr<PyObject, Deleter::Func>(Object<T>::ToPython(m_value), &Deleter::Borrow);
        }

    private:
        T m_value;
    };
}
