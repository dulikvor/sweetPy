#pragma once

#include <Python.h>
#include <unordered_map>
#include "../Core/Exception.h"
#include "CPythonType.h"

namespace sweetPy{
    class TypesContainer
    {
    private:
        typedef std::unordered_map<std::size_t, std::reference_wrapper<CPythonType>> Types;
        
    public:
        static TypesContainer& instance()
        {
            static TypesContainer instance;
            return instance;
        }
        ~TypesContainer() = default;
        void add_type(std::size_t hash_code_key, CPythonType& type, bool force = false)
        {
            if(force)
                m_types.erase(hash_code_key);
            
            m_types.insert({hash_code_key, type});
        }
        CPythonType& get_type(std::size_t hash_code_key)
        {
            auto it = m_types.find(hash_code_key);
            if(it == m_types.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Requested type - %d, wasn't found", hash_code_key);
            return it->second.get();
        }

    private:
        Types m_types;
    };
}
