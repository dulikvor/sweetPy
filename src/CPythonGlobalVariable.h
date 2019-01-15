#pragma once

#include <Python.h>
#include <string>
#include <type_traits>
#include "Core/Deleter.h"
#include "CPythonModule.h"
#include "CPythonObject.h"
#include "CPythonVariable.h"

namespace sweetPy {
    
    class CPythonGlobalVariable {
    public:
        
        template<typename T, typename T_Decay = typename std::decay<T>::type,
                typename std::enable_if<Object<T_Decay>::IsSimpleObjectType == false, bool>::type = true>
        CPythonGlobalVariable(CPythonModule &module, const std::string &name, T&& value)
                :m_module(module), m_variable(new CPythonTypedVariable<T_Decay>(name, std::forward<T>(value)))
        {
        }
        
        template<typename T, typename T_Decay = typename std::decay<T>::type,
                typename std::enable_if<Object<T_Decay>::IsSimpleObjectType == true, bool>::type = true>
        CPythonGlobalVariable(CPythonModule &module, const std::string &name, T&& value)
                :m_module(module), m_variable(new CPythonTypedVariable<T_Decay>(name, std::forward<T>(value)))
        {
            CPythonObject<T_Decay>{m_module};
        }
        
        ~CPythonGlobalVariable() {
            m_module.AddVariable(std::move(m_variable));
        }
    
    private:
        CPythonModule &m_module;
        std::unique_ptr<CPythonVariable> m_variable;
    };
}

