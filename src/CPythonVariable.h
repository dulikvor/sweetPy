#pragma once

#include <type_traits>
#include <string>
#include <Python.h>
#include "Core/Deleter.h"
#include "CPythonObject.h"

namespace sweetPy {
    
    class CPythonVariable {
    public:
        CPythonVariable(const std::string &name) : m_name(name) {}
        
        virtual ~CPythonVariable() {}
        virtual std::unique_ptr <PyObject, Deleter::Func> ToPython() const = 0;
        const std::string &GetName() const { return m_name; }
    
    private:
        std::string m_name;
    };
    
    template<typename T>
    class CPythonTypedVariable : public CPythonVariable
    {
    public:
        template<typename Y>
        CPythonTypedVariable(const std::string &name, Y&& value)
                : CPythonVariable(name), m_value(value) {}
        
        std::unique_ptr <PyObject, Deleter::Func> ToPython() const override {
            return std::unique_ptr<PyObject, Deleter::Func>(Object<T>::ToPython(m_value), &Deleter::Owner);
        }
    
    private:
        T m_value;
    };
}

