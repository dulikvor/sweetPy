#pragma once

#include <Python.h>
#include <string>
#include "../Core/Assert.h"
#include "../Core/Exception.h"
#include "../Detail/CPythonObject.h"
#include "ObjectPtr.h"

namespace sweetPy {
    
class AsciiString {
public:
    AsciiString() = default;
    explicit AsciiString(const std::string &str):m_str(str) {}
    explicit AsciiString(PyObject *unicodeStr)
    {
        CPYTHON_VERIFY(Py_TYPE(unicodeStr) == &PyUnicode_Type, "Received object is not a unicode object");
        ObjectPtr bytesObject(PyUnicode_AsASCIIString(unicodeStr), &Deleter::Owner);
        CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
        m_str = PyBytes_AsString(bytesObject.get());
    }
    AsciiString(const sweetPy::AsciiString &obj): m_str(obj.m_str) {}
    AsciiString& operator=(const sweetPy::AsciiString &rhs) {m_str = rhs.m_str; return *this;}
    AsciiString(sweetPy::AsciiString &&obj):m_str(std::move(obj.m_str)){}
    AsciiString& operator=(sweetPy::AsciiString &&rhs) {m_str = std::move(rhs.m_str); return *this;}
    
    const std::string& get_str() const { return m_str; }
    operator const std::string&() const {return m_str;}
    operator std::string() const {return m_str;}
    PyObject* to_python() const
    {
        return PyUnicode_FromString(m_str.c_str());
    }

private:
    std::string m_str;
};

template<>
struct Object<AsciiString>
{
public:
    typedef PyObject* FromPythonType;
    typedef AsciiString Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    static AsciiString get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        if(PyUnicode_CheckExact(object))
        {
            new(toBuffer)AsciiString(object);
            return *reinterpret_cast<AsciiString*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<AsciiString>& refObject = ClazzObject<ReferenceObject<AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const AsciiString>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const AsciiString>& refObject = ClazzObject<ReferenceObject<const AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "AsciiString can only originates from ref const AsciiString type or unicode object");
    }
    static AsciiString from_python(PyObject* object)
    {
        GilLock lock;
        if(PyUnicode_CheckExact(object))
        {
            return AsciiString(object);
        }
        else if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(object))
        {
            ReferenceObject<AsciiString>& refObject = ClazzObject<ReferenceObject<AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const AsciiString>>::is_ref(object))
        {
            ReferenceObject<const AsciiString>& refObject = ClazzObject<ReferenceObject<const AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "AsciiString can only originates from ref const AsciiString type or unicode object");
    }
    static PyObject* to_python(const AsciiString& value)
    {
        return value.to_python();
    }
};

template<>
struct Object<const AsciiString&>
{
public:
    typedef PyObject* FromPythonType;
    typedef AsciiString Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    
    static const AsciiString& get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        if(PyUnicode_CheckExact(object))
        {
            new(toBuffer)AsciiString(object);
            return *reinterpret_cast<AsciiString*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<AsciiString>& refObject = ClazzObject<ReferenceObject<AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const AsciiString>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const AsciiString>& refObject = ClazzObject<ReferenceObject<const AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "AsciiString can only originates from ref const AsciiString type or unicode object");
    }
    static const AsciiString& from_python(PyObject* object)
    {
        GilLock lock;
        if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(object))
        {
            ReferenceObject<AsciiString>& refObject = ClazzObject<ReferenceObject<AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const AsciiString>>::is_ref(object))
        {
            ReferenceObject<const AsciiString>& refObject = ClazzObject<ReferenceObject<const AsciiString>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "AsciiString can only originates from ref const AsciiString type or unicode object");
    }
    static PyObject* to_python(const AsciiString& value)
    {
        return ReferenceObject<const AsciiString>::alloc(value);
    }
};

template<std::size_t I>
struct ObjectWrapper<const AsciiString&, I>
{
    typedef typename Object<const AsciiString&>::FromPythonType FromPythonType;
    typedef typename Object<const AsciiString&>::Type Type;
    static void* destructor(char* buffer)
    {
        if(*reinterpret_cast<std::uint32_t*>(buffer) != MAGIC_WORD)
        {
            Type* typedPtr = reinterpret_cast<Type*>(buffer);
            typedPtr->~Type();
        }
        return nullptr;
    }
};
}
