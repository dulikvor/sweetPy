#pragma once

#include "src/Detail/MemberAccessor.h"
#include "src/Detail/ClazzPyType.h"
#include "src/Detail/CPythonObject.h"

namespace sweetPy{
    template<typename T>
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
            T &member = *(T*) ((char *) object + m_offset);
            if (ClazzObject<T>::is_ref(rhs))
            {
                T& _rhs = Object<T&>::from_python(rhs);
                member = _rhs;
            }
            else
            {
                auto _rhs = Object<T>::from_python(rhs);
                member = _rhs;
            }
        }
        
        PyObject* get(PyObject *object) override
        {
            T& member = *(T*) ((char *) object + m_offset);
            return Object<T>::to_python(member);
        }
    
    private:
        int m_offset;
    };
}