#pragma once

#include <string>
#include <memory>
#include <type_traits>
#include <Python.h>
#include <structmember.h>

namespace pycppconn{

    #define PyArgumentsTypes short, int, long, float, double, char*, short , char, char,  \
        unsigned char, unsigned short, unsigned int, unsigned long, char*, bool


    template<typename X, typename... Args>
    struct PyTypeId{};

    template<typename X>
    struct PyTypeId<X>
    {
        const static int value = -1;
    };


    template<typename X, typename... Args>
    struct PyTypeId<X, X, Args...>
    {
        const static int value = 0;
    };

    template<typename X, typename T, typename... Args>
    struct PyTypeId<X, T, Args...>
    {
        const static int value = PyTypeId<X, Args...>::value != -1 ? PyTypeId<X, Args...>::value + 1 : -1;
    };

    template<typename Type, typename MemberType>
    inline int GetOffset(MemberType Type::* member){
        return (char*)&(((Type*)nullptr)->*member) - (char*)((Type*)nullptr) + sizeof(PyObject);
    }

    class ICPythonMember{
    public:
        virtual std::unique_ptr<PyMemberDef> ToPython() const = 0;

    };


    template<typename Type, typename MemberType, typename = void>
    class CPythonMember : public ICPythonMember
    {
        std::unique_ptr<PyMemberDef> ToPython() const override{}
    };

    template<typename Type, typename MemberType>
    class CPythonMember<Type, MemberType, typename std::enable_if<!(std::is_reference<MemberType>::value || std::is_pointer<MemberType>::value)>::type> : public ICPythonMember{
    public:
        static constexpr bool IsNonConst = std::is_same<MemberType, typename std::remove_const<MemberType>::type>::value;
        CPythonMember(const std::string& name, MemberType Type::*& member, const std::string& doc):m_offset(GetOffset(member)),
         m_typeId(PyTypeId<MemberType, PyArgumentsTypes>::value == -1 ? T_OBJECT : PyTypeId<MemberType, PyArgumentsTypes>::value), m_name(name), m_doc(doc){
            static_assert(PyTypeId<short, PyArgumentsTypes>::value == T_SHORT, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<int, PyArgumentsTypes>::value == T_INT, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<long, PyArgumentsTypes>::value == T_LONG, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<float, PyArgumentsTypes>::value == T_FLOAT, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<double, PyArgumentsTypes>::value == T_DOUBLE, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<char*, PyArgumentsTypes>::value == T_STRING, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<std::string, PyArgumentsTypes>::value == -1, "Unrelated representation of type id between self and python"); //unsupported native type
            static_assert(PyTypeId<int*, PyArgumentsTypes>::value == -1, "Unrelated representation of type id between self and python"); //By pointer plain old data
            static_assert(PyTypeId<char, PyArgumentsTypes>::value == T_CHAR, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<unsigned char, PyArgumentsTypes>::value == T_UBYTE, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<unsigned short, PyArgumentsTypes>::value == T_USHORT, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<unsigned int, PyArgumentsTypes>::value == T_UINT, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<unsigned long, PyArgumentsTypes>::value ==T_ULONG, "Unrelated representation of type id between self and python");
            static_assert(PyTypeId<bool, PyArgumentsTypes>::value == T_BOOL, "Unrelated representation of type id between self and python");

        }
        std::unique_ptr<PyMemberDef> ToPython() const override{
            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                                                        const_cast<char *>(m_name.c_str()),
                                                        m_typeId,
                                                        m_offset,
                                                        IsNonConst ? 0 : READONLY,
                                                        const_cast<char *>(m_doc.c_str())
            }); //Python emphasis the use of implicit conversion of C++ string literals to prvalue of char*, so const_cast is safe.
        }

    private:
        std::string m_name;
        std::string m_doc;
        int m_offset;
        int m_typeId;
    };


    template<typename Type, typename MemberType>
    class CPythonMember<Type, MemberType, typename std::enable_if<std::is_same<MemberType, const char*>::value || std::is_same<MemberType, char*>::value>::type> : public ICPythonMember{
    public:
        static constexpr bool IsNonConst = std::is_same<MemberType, typename std::remove_const<MemberType>::type>::value;
        CPythonMember(const std::string& name, MemberType Type::*& member, const std::string& doc):m_offset(GetOffset(member)), m_name(name), m_doc(doc){}
        std::unique_ptr<PyMemberDef> ToPython() const override{
            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                    const_cast<char *>(m_name.c_str()),
                    T_STRING,
                    m_offset,
                    IsNonConst ? 0 : READONLY,
                    const_cast<char *>(m_doc.c_str())
            }); //Python emphasis the use of implicit conversion of C++ string literals to prvalue of char*, so const_cast is safe.
        }

    private:
        std::string m_name;
        std::string m_doc;
        int m_offset;
    };
}
