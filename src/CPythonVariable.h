#pragma once

#include <type_traits>
#include <string>
#include <Python.h>
#include "src/Core/Deleter.h"
#include "ICPythonVariable.h"
#include "CPythonObject.h"

namespace sweetPy {

    //Since a copy is due to happen we only want to support pod types to reduce overhead (huge pod structs are unlikeable)
    //If an array is received it will be decayed into pointer, no copy intialization is supported via array.
    template<typename T, typename Type = typename std::conditional<std::is_array<T>::value, typename std::decay<T>::type, T>::type, typename = typename std::enable_if<std::is_pod<Type>::value>::type>
    class CPythonVariable : public ICPythonVariable
    {
    public:
        template<typename Y, typename = typename std::enable_if<std::is_convertible<Y, Type>::value>::type>
        CPythonVariable(const std::string &name, Y&& value)
                : ICPythonVariable(name), m_value(value) {}

        std::unique_ptr <PyObject, Deleter::Func> ToPython() const override {
            return std::unique_ptr<PyObject, Deleter::Func>(Object<Type>::ToPython(m_value), &Deleter::Owner);
        }

    private:
        Type m_value;
    };

    template<typename T>
    class CPythonVariable<T, const char*, void> : public ICPythonVariable
    {
    public:
        template<typename Y, typename = typename std::enable_if<std::is_convertible<Y, std::string>::value>::type>
        CPythonVariable(const std::string &name, Y&& value)
                : ICPythonVariable(name), m_value(value) {}

        std::unique_ptr <PyObject, Deleter::Func> ToPython() const override
        {
            return std::unique_ptr<PyObject, Deleter::Func>(Object<std::string>::ToPython(m_value), &Deleter::Owner);
        }

    private:
        std::string m_value;
    };
}
