#pragma once

#include "src/Detail/MemberAccessor.h"
#include "src/Detail/ClazzPyType.h"
#include "src/Detail/CPythonObject.h"

namespace sweetPy{
    template<typename Type, typename MemberT>
    class TypedMemberAccessor : public MemberAccessor
    {
    public:
        explicit TypedMemberAccessor(int offset) : m_offset(offset) {}
        virtual ~TypedMemberAccessor() = default;
        
        /*
         * Set will receive the rhs object, the object may be a reference wrapper
         * /a python supported type/a C++ type.
         */
        void set(PyObject *object, PyObject *rhs) override
        {
            MemberT &member = *(MemberT*) (reinterpret_cast<char*>(&ClazzObject<Type>::get_val(object)) + m_offset);
            if (ClazzObject<ReferenceObject<MemberT>>::is_ref(rhs))
            {
                MemberT& _rhs = Object<MemberT&>::from_python(rhs);
                member = _rhs;
            }
            else
            {
                auto _rhs = Object<MemberT>::from_python(rhs);
                member = _rhs;
            }
        }
        
        PyObject* get(PyObject *object) override
        {
            MemberT& member = *(MemberT*) (reinterpret_cast<char*>(&ClazzObject<Type>::get_val(object)) + m_offset);
            return Object<MemberT>::to_python(member);
        }
    
    private:
        int m_offset;
    };
}