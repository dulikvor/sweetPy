#pragma once

#include <Python.h>
#include <unordered_map>
#include "MetaClass.h"
#include "Function.h"

namespace sweetPy{
    class ModuleContext
    {
    public:
        typedef int HashKey;
        typedef int Offset;
        typedef std::shared_ptr<Function> FunctionPtr;
        void add_function(HashKey key, FunctionPtr&& function)
        {
            if(m_functions.find(key) != m_functions.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "function key already in use");
            m_functions.insert({key, std::move(function)});
        }
        Function &get_function(HashKey key) const
        {
            auto it = m_functions.find(key);
            if (it == m_functions.end())
                throw CPythonException(PyExc_LookupError, __CORE_SOURCE,
                                       "Requested function couldn't be found, key - %d", key);
            return *it->second;
        }
    
    private:
        typedef std::unordered_map<HashKey, FunctionPtr> Functions;
        Functions m_functions;
    };
}


