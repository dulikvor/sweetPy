#pragma once

#include <string>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <Python.h>
#include <structmember.h>
#include "../Core/Traits.h"

namespace sweetPy{

    #define PyArgumentsTypes short, int, long, float, double, char*, short , char, char,  \
        unsigned char, unsigned short, unsigned int, unsigned long, char*, bool

    template<typename Type, typename MemberType>
    inline int get_offset(MemberType Type::* member){
        return (char*)&(((Type*)nullptr)->*member) - (char*)((Type*)nullptr);
    }

    class Member
    {
    public:
        typedef std::unique_ptr<PyMemberDef> MemberDefPtr;
        virtual ~Member() = default;
        virtual MemberDefPtr to_python() const = 0;

    };

    template<typename Type, typename MemberType, typename = void>
    class TypedMember : public Member
    {
        MemberDefPtr to_python() const override{ return MemberDefPtr(nullptr); }
    };

    template<typename Type, typename MemberType>
    class TypedMember<Type, MemberType,
            enable_if_t<!(std::is_reference<MemberType>::value ||
                            std::is_pointer<MemberType>::value)>>
        : public Member
    {
    public:
        static constexpr bool IsNonConst = std::is_same<MemberType, typename std::remove_const<MemberType>::type>::value;
        TypedMember(const std::string& name, MemberType Type::*& member, const std::string& doc):m_offset(get_offset(member)),
         m_typeId(py_type_id<MemberType, PyArgumentsTypes>::value == -1 ? T_OBJECT : py_type_id<MemberType, PyArgumentsTypes>::value), m_name(name), m_doc(doc){
            static_assert(py_type_id<short, PyArgumentsTypes>::value == T_SHORT, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<int, PyArgumentsTypes>::value == T_INT, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<long, PyArgumentsTypes>::value == T_LONG, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<float, PyArgumentsTypes>::value == T_FLOAT, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<double, PyArgumentsTypes>::value == T_DOUBLE, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<std::string, PyArgumentsTypes>::value == -1, "Unrelated representation of type id between self and python"); //unsupported native type
            static_assert(py_type_id<int*, PyArgumentsTypes>::value == -1, "Unrelated representation of type id between self and python"); //By pointer plain old data
            static_assert(py_type_id<char, PyArgumentsTypes>::value == T_CHAR, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<unsigned char, PyArgumentsTypes>::value == T_UBYTE, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<unsigned short, PyArgumentsTypes>::value == T_USHORT, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<unsigned int, PyArgumentsTypes>::value == T_UINT, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<unsigned long, PyArgumentsTypes>::value ==T_ULONG, "Unrelated representation of type id between self and python");
            static_assert(py_type_id<bool, PyArgumentsTypes>::value == T_BOOL, "Unrelated representation of type id between self and python");

        }
        MemberDefPtr to_python() const override
        {
            char* name = new char[m_name.length() + 1];
            std::copy_n(m_name.c_str(), m_name.length(), name);
            name[m_name.length()] = '\0';

            char* doc = new char[m_doc.length() + 1];
            std::copy_n(m_doc.c_str(), m_doc.length(), doc);
            doc[m_doc.length()] = '\0';

            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                                                        name,
                                                        m_typeId,
                                                        m_offset,
                                                        IsNonConst ? 0 : READONLY,
                                                        doc
            }); //Python emphasis the use of implicit conversion of C++ string literals to prvalue of char*, so const_cast is safe.
        }

    private:
        int m_offset;
        int m_typeId;
        std::string m_name;
        std::string m_doc;
    };


    template<typename Type, typename MemberType>
    class TypedMember<Type, MemberType, enable_if_t<
                        std::is_same<MemberType, const char*>::value ||
                        std::is_same<MemberType, char*>::value>>
            : public Member
    {
    public:
        static constexpr bool IsNonConst = std::is_same<MemberType, typename std::remove_const<MemberType>::type>::value;
        TypedMember(const std::string& name, MemberType Type::*& member, const std::string& doc):m_name(name), m_doc(doc), m_offset(get_offset(member))
        {
            static_assert(py_type_id<char*, PyArgumentsTypes>::value == T_STRING, "Unrelated representation of type id between self and python");
        }
        MemberDefPtr to_python() const override
        {
            char* name = new char[m_name.length() + 1];
            std::copy_n(m_name.c_str(), m_name.length(), name);
            name[m_name.length()] = '\0';

            char* doc = new char[m_doc.length() + 1];
            std::copy_n(m_doc.c_str(), m_doc.length(), doc);
            doc[m_doc.length()] = '\0';

            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                    name,
                    T_STRING,
                    m_offset,
                    IsNonConst ? 0 : READONLY,
                    doc
            }); //Python emphasis the use of implicit conversion of C++ string literals to prvalue of char*, so const_cast is safe.
        }

    private:
        std::string m_name;
        std::string m_doc;
        int m_offset;
    };
}
