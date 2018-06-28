#pragma once

#include "IMemberAccessor.h"
#include "CPythonRef.h"
#include "CPythonObject.h"

namespace pycppconn {

    template<typename MemberType>
    class MemberAccessor : public IMemberAccessor {
    public:
        MemberAccessor(int offset) : m_offset(offset) {}

        virtual ~MemberAccessor() {}

        /*
         * Set will receive the rhs object, the object may be a reference wrapper
         * /a python supported type/a C++ type.
         */
        void Set(PyObject *object, PyObject *rhs) override {
            MemberType &member = *(MemberType *) ((char *) object + m_offset);
            if (CPythonRef<>::IsReferenceType<MemberType>(rhs)) {
                MemberType &_rhs = Object<MemberType &>::FromPython(rhs);
                member = _rhs;
            } else {
                auto _rhs = Object<MemberType>::FromPython(rhs);
                member = _rhs;
            }
        }

    private:
        int m_offset;
    };
}
