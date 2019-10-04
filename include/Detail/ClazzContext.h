#pragma once

#include <unordered_map>
#include <memory>
#include "../Core/Exception.h"
#include "MemberAccessor.h"
#include "Function.h"

namespace sweetPy{
    class ClazzContext
    {
    public:
        typedef int HashKey;
        typedef int Offset;
        typedef std::shared_ptr<Function> FunctionPtr;
        typedef std::shared_ptr<MemberAccessor> MemberAccessorPtr;
        typedef std::unordered_map<HashKey, FunctionPtr> MemberFunctions;
        
        void add_member(Offset offset, MemberAccessorPtr&& memberAccessor)
        {
            if (m_memberAccessors.find(offset) != m_memberAccessors.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE,
                                       "offset already defined  - %d", offset);
            m_memberAccessors.insert({offset, std::move(memberAccessor)});
        }
        MemberAccessor &get_accessor(Offset offset) const
        {
            auto it = m_memberAccessors.find(offset);
            if (it == m_memberAccessors.end())
                throw CPythonException(PyExc_LookupError, __CORE_SOURCE,
                                       "Requested accessor couldn't be found, offset - %d", offset);
            return *it->second;
        }
        void add_member_function(HashKey key, FunctionPtr&& memberFunction)
        {
            if(m_memberFunctions.find(key) != m_memberFunctions.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "member function key already in use");
            m_memberFunctions.insert({key, std::move(memberFunction)});
        }
        Function &get_member_function(HashKey key) const
        {
            auto it = m_memberFunctions.find(key);
            if (it == m_memberFunctions.end())
                throw CPythonException(PyExc_LookupError, __CORE_SOURCE,
                                       "Requested member function couldn't be found, key - %d", key);
            return *it->second;
        }
        void add_member_static_function(HashKey key, FunctionPtr&& memberFunction)
        {
            if(m_memberStaticFunctions.find(key) != m_memberStaticFunctions.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "member function key already in use");
            m_memberStaticFunctions.insert({key, std::move(memberFunction)});
        }
        bool is_member_functions_empty()
        {
            return m_memberFunctions.empty();
        }
        bool is_member_static_functions_empty()
        {
            return m_memberStaticFunctions.empty();
        }
        const MemberFunctions& get_member_functions() const { return m_memberFunctions; }
        const MemberFunctions& get_member_static_functions() const { return m_memberStaticFunctions; }
    
    private:
        typedef std::unordered_map<int, MemberAccessorPtr> MemberAccessors;
        MemberAccessors m_memberAccessors; //Nothing is shared, but due to the fact vptr is not allowed we will use raii to keep the memory in check.
        MemberFunctions m_memberFunctions;
        MemberFunctions m_memberStaticFunctions;
        
    };
}