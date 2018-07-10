#pragma once

#include <string>
#include <type_traits>
#include <Python.h>
#include "Deleter.h"
#include "CPythonModule.h"
#include "CPythonObject.h"
#include "CPythonVariable.h"

namespace sweetPy {

    class CPythonGlobalVariable {
    public:

        template<typename T>
        CPythonGlobalVariable(CPythonModule &module, const std::string &name, const std::string &doc, const T& value)
                :m_module(module), m_variable(new CPythonVariable(name, doc, value)){}

        ~CPythonGlobalVariable() {
            if( Object<T>::IsSimpleObjectType == false)
            {
                CPythonObject<T>(m_module);
            }
            m_module.AddObject(std::move(m_variable));
        }

    private:
        std::unique_ptr<ICPythonVariable> m_variable;
        CPythonModule &m_module;
    };
}
