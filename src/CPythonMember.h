#pragma once

#include <string>
#include <Python.h>
#include <type_traits>
#include <structmember.h>

namespace pycppconn{

    #define PyArgumentsTypes short, int, long, float, double, char*, short , char, char,  \
        unsigned char, unsigned short, unsigned int, unsigned long, char*, bool


    template<typename X, typename... Args>
    struct PyTypeId{};

    template<typename X>
    struct PyTypeId<X>
    {
        const static int value = 6;
    };


    template<typename X, typename... Args>
    struct PyTypeId<X, X, Args...>
    {
        const static int value = 0;
    };

    template<typename X, typename T, typename... Args>
    struct PyTypeId<X, T, Args...>
    {
        const static int value = PyTypeId<X, Args...>::value != 6 ? PyTypeId<X, Args...>::value + 1 : 6;
    };

    template<typename Type, typename MemberType>
    inline int GetOffset(MemberType Type::* member){
        return reinterpret_cast<Type*>(nullptr)->*member- reinterpret_cast<Type*>(nullptr);
    }

    template<typename Type, typename MemberType,
            typename NonConstMemberType = typename std::remove_const<MemberType>::type,
            bool IsNonConst = std::is_same<MemberType, NonConstMemberType>::value>
    class CPythonMember{
    public:
        CPythonMember(MemberType Type::* member):m_offset(GetOffset(member)),
         m_typeId(PyTypeId<MemberType, PyArgumentsTypes>::value){
            static_assert(PyTypeId<short, PyArgumentsTypes>::value, T_SHORT);
            static_assert(PyTypeId<int, PyArgumentsTypes>::value, T_INT;
            static_assert(PyTypeId<long, PyArgumentsTypes>::value, T_LONG);
            static_assert(PyTypeId<float, PyArgumentsTypes>::value, T_FLOAT);
            static_assert(PyTypeId<double, PyArgumentsTypes>::value, T_DOUBLE);
            static_assert(PyTypeId<char*, PyArgumentsTypes>::value, T_STRING);
            static_assert(PyTypeId<std::string, PyArgumentsTypes>::value, T_OBJECT);
            static_assert(PyTypeId<char, PyArgumentsTypes>::value, T_CHAR);
            static_assert(PyTypeId<unsigned char, PyArgumentsTypes>::value, T_UBYTE);
            static_assert(PyTypeId<unsigned short, PyArgumentsTypes>::value, T_USHORT);
            static_assert(PyTypeId<unsigned int, PyArgumentsTypes>::value, T_UINT);
            static_assert(PyTypeId<unsigned long, PyArgumentsTypes>::value, T_ULONG);
            static_assert(PyTypeId<bool, PyArgumentsTypes>::value, T_BOOL);
        }

    private:
        int m_offset;
        int m_typeId;
    };
}
