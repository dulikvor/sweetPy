#pragma once

#include <Python.h>
#include <string>
#include <type_traits>
#include "src/Core/Deleter.h"
#include "CPythonModule.h"
#include "CPythonObject.h"
#include "CPythonVariable.h"

namespace sweetPy {

    class CPythonGlobalVariable {
    public:

        template<typename T, typename T_NoRef = typename std::remove_reference<T>::type,
                typename std::enable_if<Object<T_NoRef>::IsSimpleObjectType == false, bool>::type = true>
        CPythonGlobalVariable(CPythonModule &module, const std::string &name, T&& value)
                :m_module(module), m_variable(new CPythonVariable<T_NoRef>(name, std::forward<T>(value)))
        {
        }

        template<typename T, typename T_NoRef = typename std::remove_reference<T>::type,
                typename std::enable_if<Object<T_NoRef>::IsSimpleObjectType == true, bool>::type = true>
        CPythonGlobalVariable(CPythonModule &module, const std::string &name, T&& value)
                :m_module(module), m_variable(new CPythonVariable<T_NoRef>(name, std::forward<T>(value)))
        {
            CPythonObject<T_NoRef>{m_module};
        }

        ~CPythonGlobalVariable() {
            m_module.AddVariable(std::move(m_variable));
        }

    private:
        std::unique_ptr<ICPythonVariable> m_variable;
        CPythonModule &m_module;
    };
}
