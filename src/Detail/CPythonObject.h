#pragma once

#include <Python.h>
#include <cstdint>
#include <datetime.h>
#include <string>
#include <vector>
#include <type_traits>
#include "core/Source.h"
#include "Core/Exception.h"
#include "Core/Traits.h"
#include "Core/Lock.h"
#include "Core/Assert.h"
#include "Core/Deleter.h"
#include "TypesContainer.h"
#include "src/Detail/CPythonEnumValue.h"
#include "CPythonType.h"
#include "src/Detail/ClazzPyType.h"
#include "Object.h"

namespace sweetPy{
    class Module;
    static std::uint32_t MAGIC_WORD = 0xABBACDDC;

    template<typename T, typename = void>
    struct Object{};

    template<typename T>
    struct Object<T, enable_if_t<!std::is_pointer<T>::value && !is_container<T>::value && std::is_copy_constructible<T>::value &&
                                             !std::is_enum<T>::value && !std::is_reference<T>::value>> {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = true;
        static T get_typed(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const T>>::is_ref(object))
            {
                ReferenceObject<const T>& refObject = ClazzObject<ReferenceObject<const T>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                new(toBuffer)T(*(T*)object);
                return *reinterpret_cast<T*>(toBuffer);
            }
        }
        static T from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const T>>::is_ref(object))
            {
                ReferenceObject<const T>& refObject = ClazzObject<ReferenceObject<const T>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                return ClazzObject<T>::get_val(object);
            }
        }
        template<typename X = Type, typename = enable_if_t<std::is_copy_constructible<X>::value>>
        static PyObject* to_python(T& value)
        {
            return ValueObject<T>::alloc(value);
        }
        template<typename X = Type, typename = enable_if_t<std::is_move_constructible<X>::value>>
        static PyObject* to_python(T&& value)
        {
            return ValueObject<T>::alloc(std::move(value));
        }
        template<typename X = Type, typename = enable_if_t<std::is_copy_constructible<X>::value>>
        static PyObject* to_python(const T& value)
        {
            return ValueObject<T>::alloc(value);
        }
    };

    template<typename T>
    struct Object<T, enable_if_t<!std::is_pointer<T>::value && !std::is_copy_constructible<T>::value &&
                                             !std::is_enum<T>::value && !std::is_reference<T>::value &&
                                             std::is_move_constructible<T>::value>>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = true;
        static T get_typed(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
            {
                auto& value = ClazzObject<T>::get_val(object);
                new(toBuffer)T(std::move(value));
                return std::move(*reinterpret_cast<T*>(toBuffer));
            }
        }
        static T from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                return std::move(ClazzObject<T>::get_val(object));
            }
        }
        template<typename X = T, typename = enable_if_t<std::is_move_constructible<X>::value>>
        static PyObject* to_python(T&& value)
        {
            return ValueObject<T>::alloc(std::move(value));
        }
    };

    template<typename T>
    struct Object<T, enable_if_t<std::is_enum<T>::value>>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = false;
        static T get_typed(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            ObjectPtr attrName(PyUnicode_FromString("value"), &Deleter::Owner);
            ObjectPtr attr(PyObject_GetAttr(object, attrName.get()), &Deleter::Owner);
            new(toBuffer)T((T)int(PyLong_AsLongLong(attr.get())));
            return *reinterpret_cast<T*>(toBuffer);
        }
    };

    template<typename T>
    struct Object<T&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef void* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static T& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                return ClazzObject<T>::get_val(object);
            }
        }
        static T& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                return ClazzObject<T>::get_val(object);
            }
        }
        static PyObject* to_python(T& value)
        {
            return ReferenceObject<T>::alloc(value);
        }
    };

    template<typename T>
    struct Object<const T&>{
    public:
        typedef PyObject* FromPythonType;
        typedef void* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const T& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const T>>::is_ref(object))
            {
                ReferenceObject<const T>& refObject = ClazzObject<ReferenceObject<const T>>::get_val(object);
                return refObject.get_ref();
            }
            else
                return ClazzObject<T>::get_val(object);
        }
        static const T& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const T>>::is_ref(object))
            {
                ReferenceObject<const T>& refObject = ClazzObject<ReferenceObject<const T>>::get_val(object);
                return refObject.get_ref();
            }
            else
                return ClazzObject<T>::get_val(object);
        }
        static PyObject* to_python(const T& value)
        {
            return ReferenceObject<const T>::alloc(value);
        }
    };

    //All python value category are basicaly lvalue, so as long as the C++ function prototype will use reference collapsing it will receive it as lvalue.
    template<typename T>
    struct Object<T&&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef void* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static T&& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
                return std::move(ClazzObject<T>::get_val(object));
        }
        static T&& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<T>>::is_ref(object))
            {
                ReferenceObject<T>& refObject = ClazzObject<ReferenceObject<T>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
                return std::move(ClazzObject<T>::get_val(object));
        }
        //No meaning to return rvalue reference to python (only supports lvalue value category), so ToPython is not implemented.
    };

    template<typename T>
    struct Object<std::vector<T>>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::vector<T> Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static std::vector<T> get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(object->ob_type == &PyList_Type)
            {
                Py_ssize_t numOfElements = PyList_Size(object);
                new(toBuffer)std::vector<T>;
                std::vector<T>& vectorObject = *(std::vector<T>*)(toBuffer);
                vectorObject.reserve(numOfElements);
                for(int index = 0; index < numOfElements; index++)
                {
                    PyObject* element = PyList_GetItem(object, index);
                    vectorObject.emplace_back(Object<T>::from_python(element));
                }
                return *reinterpret_cast<std::vector<T>*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<Type>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<Type>& refObject = ClazzObject<ReferenceObject<Type>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Type>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const Type>& refObject = ClazzObject<ReferenceObject<const Type>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "std::vector can only originates from python list type or ref to std::vector type, const ref to std::vector type");
        }
        static std::vector<T> from_python(PyObject* object)
        {
            GilLock lock;
            if(object->ob_type == &PyList_Type)
            {
                Py_ssize_t numOfElements = PyList_Size(object);
                std::vector<T> vec;
                vec.reserve(numOfElements);
                for(int index = 0; index < numOfElements; index++)
                {
                    PyObject* element = PyList_GetItem(object, index);
                    vec.emplace_back(Object<T>::from_python(element));
                }
                return vec;
            }
            else if(ClazzObject<ReferenceObject<Type>>::is_ref(object))
            {
                ReferenceObject<Type>& refObject = ClazzObject<ReferenceObject<Type>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Type>>::is_ref(object))
            {
                ReferenceObject<const Type>& refObject = ClazzObject<ReferenceObject<const Type>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "std::vector can only originates from python list type or ref to std::vector type, const ref to std::vector type");
        }
        static PyObject* to_python(const std::vector<T>& object)
        {
            PyObject* pyListObject = PyList_New(object.size());
            for( int index = 0; index < object.size(); index++)
                PyList_SetItem(pyListObject, index, Object<T>::to_python(object[index]));

            return pyListObject;
        }
    };

    template<typename T>
    struct Object<std::vector<T>&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef void* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static std::vector<T>& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(ClazzObject<ReferenceObject<std::vector<T>>>::is_ref(object))
            {
                ReferenceObject<std::vector<T>>& refObject = ClazzObject<ReferenceObject<std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "std::vector can only originates from ref to std::vector type");
        }
        static std::vector<T>& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<std::vector<T>>>::is_ref(object))
            {
                ReferenceObject<std::vector<T>>& refObject = ClazzObject<ReferenceObject<std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python list object to std::vector is not possible");
            }
        }
        static PyObject* to_python(std::vector<T>& value)
        { //Only l_value, no xpire value
            return ReferenceObject<std::vector<T>>::alloc(value);
        }
    };

    template<typename T>
    struct Object<const std::vector<T>&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::vector<T> Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static const std::vector<T>& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(object->ob_type == &PyList_Type)
            {
                Py_ssize_t numOfElements = PyList_Size(object);
                new(toBuffer)std::vector<T>;
                std::vector<T>& vectorObject = *(std::vector<T>*)(toBuffer);
                vectorObject.reserve(numOfElements);
                for(int index = 0; index < numOfElements; index++)
                {
                    PyObject* element = PyList_GetItem(object, index);
                    vectorObject.emplace_back(Object<T>::from_python(element));
                }
                return *reinterpret_cast<std::vector<T>*>(toBuffer);
            }
            if(ClazzObject<ReferenceObject<std::vector<T>>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<std::vector<T>>& refObject = ClazzObject<ReferenceObject<std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::vector<T>>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const std::vector<T>>& refObject = ClazzObject<ReferenceObject<const std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const ref std::vector can only originates from python list type or ref to std::vector type or const ref to std::vector type");
        }
        static const std::vector<T>& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<std::vector<T>>>::is_ref(object))
            {
                ReferenceObject<std::vector<T>>& refObject = ClazzObject<ReferenceObject<std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::vector<T>>>::is_ref(object))
            {
                ReferenceObject<const std::vector<T>>& refObject = ClazzObject<ReferenceObject<const std::vector<T>>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const ref std::vector can only originates from ref to std::vector type or const ref to std::vector type");
        }
        static PyObject* to_python(const std::vector<T>& value)
        { //Only l_value, no xpire value
            return ReferenceObject<const std::vector<T>>::alloc(value);
        }
    };

    template<>
    struct Object<const char*>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef const char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static const char* get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyBytes_Type)
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                return PyBytes_AsString(object);
            }
            else if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                char* buffer = PyBytes_AsString(bytesObject.get());
                std::size_t size = strlen(buffer) + 1;
                char*& copyTo = *reinterpret_cast<char**>(toBuffer);
                copyTo = new char[size];
                std::memcpy(copyTo, buffer, size);
                return copyTo;
            }
            else if(ClazzObject<ReferenceObject<const char*>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const char*>& refObject = ClazzObject<ReferenceObject<const char*>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static const char* from_python(PyObject* object)
        {
            GilLock lock;
            if(Py_TYPE(object) == &PyBytes_Type)
                return PyBytes_AsString(object);
            else if(ClazzObject<ReferenceObject<const char*>>::is_ref(object))
            {
                ReferenceObject<const char*>& refObject = ClazzObject<ReferenceObject<const char*>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion to const char* is only allowed from python's byte array, ref const char* wrapper or ref char* wrapper");
        }
        static PyObject* to_python(const char* data)
        {
            return PyUnicode_FromString(data);
        }
    };

    template<>
    struct Object<char*>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static char* get_typed(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyBytes_Type)
                return PyBytes_AsString(object);
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static char* from_python(PyObject* object)
        {
            GilLock lock;
            if(Py_TYPE(object) == &PyBytes_Type)
                return PyBytes_AsString(object);
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion to char* is only allowed from python's byte array or ref char* wrapper");
        }
        static PyObject* to_python(char* data)
        {
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "char* is not convertable into python");
        }
    };
    
    template<>
    struct Object<char const *&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef const char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static char const *& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyBytes_Type)
            {
                char* buffer = PyBytes_AsString(object);
                std::size_t size = strlen(buffer) + 1;
                char*& copyTo = *reinterpret_cast<char**>(toBuffer);
                copyTo = new char[size];
                std::memcpy(copyTo, buffer, size);
                return const_cast<char const *&>(copyTo);
            }
            else if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                char* buffer = PyBytes_AsString(bytesObject.get());
                std::size_t size = strlen(buffer) + 1;
                char*& copyTo = *reinterpret_cast<char**>(toBuffer);
                copyTo = new char[size];
                std::memcpy(copyTo, buffer, size);
                return const_cast<char const *&>(copyTo);
            }
            else if(ClazzObject<ReferenceObject<const char*>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const char*>& refObject = ClazzObject<ReferenceObject<const char*>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return const_cast<char const *&>(refObject.get_ref());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "c-type string ref can only originate from - pyUnicode, pyBytes, ref c-type str type, ref const c-type str type");
        }
        static char const *& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const char*>>::is_ref(object))
            {
                ReferenceObject<const char*>& refObject = ClazzObject<ReferenceObject<const char*>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                return const_cast<char const *&>(refObject.get_ref());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "c-type string ref can only originate from - ref c-type str type, ref const c-type str type");
        }
        static PyObject* to_python(const char*& value)
        {
            return ReferenceObject<const char*>::alloc(value);
        }
    };

    template<size_t N>
    struct Object<char[N]>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
    };

    template<size_t N>
    struct Object<const char[N]>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
    };

    template<size_t N>
    struct Object<char(&)[N]>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static char (&get_typed(char* fromBuffer, char* toBuffer))[N]
        {
            PyObject* object = *(PyObject**)fromBuffer;
            //Conversion from python unicode and bytecode will be supported due to limitation of scope of returned value.
            if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                CPYTHON_VERIFY(strlen(refObject.get_ref()) <= N-1, "Python string size is too long for char array.");
                return (char(&)[N])(refObject.get_ref());
            }
            else if(ClazzObject<ReferenceObject<char[N]>>::is_ref(object))
            {
                ReferenceObject<char[N]>& refObject = ClazzObject<ReferenceObject<char[N]>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "char array can only originates from python's unicode string, bytes array or integral types - char* or char[N]");
        }
        static char(&from_python(PyObject* object))[N]
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<char[N]>>::is_ref(object))
            {
                ReferenceObject<char[N]>& refObject = ClazzObject<ReferenceObject<char[N]>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Only char[N]& is supported.");
        }
        static PyObject* to_python(char(&value)[N])
        {
            return ReferenceObject<char[N]>::alloc(value);
        }
    };

    template<size_t N>
    struct Object<const char(&)[N]> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static const char (&get_typed(char* fromBuffer, char*&& toBuffer))[N]
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                char* bytesBuffer = PyBytes_AsString(bytesObject.get());

                size_t length = strlen(bytesBuffer);
                CPYTHON_VERIFY(length <= N-1, "Python string size is too long for char array.");
                toBuffer = new char[N];
                memcpy(toBuffer, bytesBuffer, length);
                toBuffer[length] = '\0';

                return (const char(&)[N])toBuffer;
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                CPYTHON_VERIFY(length <= N, "Python string size is too long for char array.");
                char* bytesBuffer = PyBytes_AsString(object);

                toBuffer = new char[N];
                memcpy(toBuffer, bytesBuffer, length);
                return (const char(&)[N])toBuffer;
            }
            if(ClazzObject<ReferenceObject<char*>>::is_ref(object))
            {
                ReferenceObject<char*>& refObject = ClazzObject<ReferenceObject<char*>>::get_val(object);
                CPYTHON_VERIFY(strlen(refObject.get_ref()) <= N-1, "Python string size is too long for char array.");
                return (const char(&)[N])(refObject.get_ref());
            }
            else if(ClazzObject<ReferenceObject<const char*>>::is_ref(object))
            {
                ReferenceObject<const char*>& refObject = ClazzObject<ReferenceObject<const char*>>::get_val(object);
                CPYTHON_VERIFY(strlen(refObject.get_ref()) <= N-1, "Python string size is too long for char array.");
                return (const char(&)[N])(refObject.get_ref());
            }
            else if(ClazzObject<ReferenceObject<char[N]>>::is_ref(object))
            {
                ReferenceObject<char[N]>& refObject = ClazzObject<ReferenceObject<char[N]>>::get_val(object);
                return (const char(&)[N])(refObject.get_ref());
            }
            else if(ClazzObject<ReferenceObject<const char[N]>>::is_ref(object))
            {
                ReferenceObject<const char[N]>& refObject = ClazzObject<ReferenceObject<const char[N]>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "char array can only originates from python's unicode string, bytes array or integral types - char*, const char*, const char[N] or char[N]");
        }
        static const char(&from_python(PyObject* object))[N]
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const char[N]>>::is_ref(object))
            {
                ReferenceObject<const char[N]>& refObject = ClazzObject<ReferenceObject<const char[N]>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Only const char[N]& is supported.");
        }
        static PyObject* to_python(const char(&value)[N])
        {
            return ReferenceObject<const char[N]>::alloc(value);
        }
    };

    template<>
    struct Object<std::string>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::string>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const std::string>& refObject = ClazzObject<ReferenceObject<const std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static std::string from_python(PyObject* object)
        {
            GilLock lock;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                return std::string(PyBytes_AsString(bytesObject.get()));
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                return std::string(PyBytes_AsString(object), length);
            }
            else if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::string>>::is_ref(object))
            {
                ReferenceObject<const std::string>& refObject = ClazzObject<ReferenceObject<const std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion is only legit from python's unicode string, bytes array or std::string, const std::string ref wrapper object");
        }
        static PyObject* to_python(const std::string& value)
        {
            return PyBytes_FromString(value.c_str());
        }
    };
    //Providing lvalue string is not possible, due to transform between two types, so from_python is removed.
    template<>
    struct Object<const std::string&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const std::string& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::string>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const std::string>& refObject = ClazzObject<ReferenceObject<const std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const string& can only originates from python's unicode string, bytes array or ref string, ref const string type");
        }
        static const std::string& from_python(PyObject* object)
        {
            if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const std::string>>::is_ref(object))
            {
                ReferenceObject<const std::string>& refObject = ClazzObject<ReferenceObject<const std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else{
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python string to const string& is not possible");
            }
        }
        static PyObject* to_python(const std::string& value)
        {
            return ReferenceObject<const std::string>::alloc(value);
        }
    };

    template<>
    struct Object<std::string&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from ref string type");
        }
        static std::string& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from ref string wrapper");
        }
        static PyObject* to_python(std::string& value)
        {
            return ReferenceObject<std::string>::alloc(value);
        }
    };

    template<>
    struct Object<std::string&&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string&& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                ObjectPtr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(ClazzObject<ReferenceObject<std::string>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<std::string>& refObject = ClazzObject<ReferenceObject<std::string>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& can only originates from python string or ref string wrapper");
        }
        static std::string&& from_python(PyObject* object)
        {
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& is not supported");
        }
        static PyObject* to_python(std::string&& data)
        {
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Transform of to be expired value to python is not possible");
        }
    };

    template<>
    struct Object<bool>
    {
    public:
        typedef long FromPythonType;
        typedef bool Type;
        static constexpr const char *Format = "l";
        static const bool IsSimpleObjectType = false;
        static bool& get_typed(char* fromBuffer, char* toBuffer)
        {
            new(toBuffer)bool(*reinterpret_cast<bool*>(fromBuffer));
            return *reinterpret_cast<bool*>(toBuffer);
        }
        static bool from_python(PyObject* obj)
        {
            GilLock lock;
            return (bool)PyLong_AsLong(obj);
        }
        static PyObject* to_python(const bool& data)
        {
            return PyBool_FromLong(data);
        }
    };

    template<>
    struct Object<double>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef double Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static double get_typed(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyFloat_Type)
            {
                new(toBuffer)double(PyFloat_AsDouble(object));
                return *reinterpret_cast<double*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)double(PyLong_AsDouble(object));
                return *reinterpret_cast<double*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const double>>::is_ref(object))
            {
                ReferenceObject<const double>& refObject = ClazzObject<ReferenceObject<const double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "double can only originates from ref double type, ref const double type, python float object, ref int type, ref const int type, python long type");
        }
        static double from_python(PyObject* object)
        {
            GilLock lock;
            if(Py_TYPE(object) == &PyFloat_Type)
            {
                return PyFloat_AsDouble(object);
            }
            else if(Py_TYPE(object) == &PyLong_Type)
            {
                return PyLong_AsDouble(object);
            }
            else if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const double>>::is_ref(object))
            {
                ReferenceObject<const double>& refObject = ClazzObject<ReferenceObject<const double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "double can only originates from ref double type, ref const double type, python float object, ref int type, ref const int type, python long object");
        }

        static PyObject* to_python(const double& data)
        {
            return PyFloat_FromDouble(data);
        }
    };

    template<>
    struct Object<const double&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef double Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const double& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyFloat_Type)
            {
                new(toBuffer)double(PyFloat_AsDouble(object));
                return *reinterpret_cast<double*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)double(PyLong_AsDouble(object));
                return *reinterpret_cast<double*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const double>>::is_ref(object))
            {
                ReferenceObject<const double>& refObject = ClazzObject<ReferenceObject<const double>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Invalid conversion to native const double&");
        }
        //Conversion from python float to const double& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static const double& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const double>>::is_ref(object))
            {
                ReferenceObject<const double>& refObject = ClazzObject<ReferenceObject<const double>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Invalid conversion to native const double&");
        }
        static PyObject* to_python(const double& value)
        {
            return ReferenceObject<const double>::alloc(value);
        }
    };

    template<>
    struct Object<double&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef double Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static double& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "double& can only originates from ref double type");
        }
        static double& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "double& can only originates from ref double type");
        }
        static PyObject* to_python(double& value)
        {
            return ReferenceObject<double>::alloc(value);
        }
    };

    template<>
    struct Object<double&&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef double Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static double&& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(ClazzObject<ReferenceObject<double>>::is_ref(object))
            {
                ReferenceObject<double>& refObject = ClazzObject<ReferenceObject<double>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "double&& can only originates from ref double type or py float type");
        }
        //Conversion from python float to double&& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static double&& from_python(PyObject* object)
        {
            GilLock lock;
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no python representation for double&&");
        }
        static PyObject* to_python(double&& data)
        {
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no conversion from double&& to python");
        }
    };

    template<>
    struct Object<int>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int get_typed(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)int(PyLong_AsLongLong(object));
                return *reinterpret_cast<int*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type, ref const int type or python long object");
        }

        static int from_python(PyObject* object)
        {
            GilLock lock;
            if(Py_TYPE(object) == &PyLong_Type)
            {
                return int(PyLong_AsLongLong(object));
            }
            else if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int can only originates from ref int type, ref const int type or python long object");
        }

        static PyObject* to_python(const int& data)
        {
            return PyLong_FromLong(data);
        }
    };

    template<>
    struct Object<const int&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const int& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)int(PyLong_AsLong(object));
                return *reinterpret_cast<int*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int wrapper type, ref const int wrapper type or python long object");
        }
        //Conversion from python long to const int& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static const int& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const int>>::is_ref(object))
            {
                ReferenceObject<const int>& refObject = ClazzObject<ReferenceObject<const int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type or ref const int type");
        }
        static PyObject* to_python(const int& value)
        {
            return ReferenceObject<const int>::alloc(value);
        }
    };

    template<>
    struct Object<int&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int& can only originates from ref int type");
        }
        static int& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int& can only originates from ref int type");
        }
        static PyObject* to_python(int& value)
        {
            return ReferenceObject<int>::alloc(value);
        }
    };

    template<>
    struct Object<int&&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int&& get_typed(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(ClazzObject<ReferenceObject<int>>::is_ref(object))
            {
                ReferenceObject<int>& refObject = ClazzObject<ReferenceObject<int>>::get_val(object);
                return std::move(refObject.get_ref());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int&& can only originates from ref int type or py long type");
        }
        //Conversion from python long to int&& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static int&& from_python(PyObject* object)
        {
            GilLock lock;
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no python representation for int&&");
        }
        static PyObject* to_python(int&& data)
        {
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no conversion from int&& to python");
        }
    };
    
    template<>
    struct Object<ObjectPtr>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef ObjectPtr Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        
        static ObjectPtr get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
    
            if(ClazzObject<ReferenceObject<const ObjectPtr>>::is_ref(object) || ClazzObject<ReferenceObject<ObjectPtr>>::is_ref(object))
            {
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "ObjectPtr is not copy constructiable");
            }
            else
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                Py_XINCREF(object);
                return ObjectPtr(object, &Deleter::Owner);
            }
        }
        static ObjectPtr from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const ObjectPtr>>::is_ref(object) || ClazzObject<ReferenceObject<ObjectPtr>>::is_ref(object))
            {
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "ObjectPtr is not copy constructiable");
            }
            else
            {
                Py_XINCREF(object);
                return ObjectPtr(object, &Deleter::Owner);
            }
        }
        static PyObject* to_python(const ObjectPtr& value)
        {
            Py_XINCREF(value.get());
            return value.get();
        }
    };
    
    template<>
    struct Object<const ObjectPtr&>
    {
    public:
        typedef PyObject *FromPythonType;
        typedef ObjectPtr Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
    
        static const ObjectPtr &get_typed(char *fromBuffer, char *toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject *object = *(PyObject **) fromBuffer;
            if(ClazzObject<ReferenceObject<ObjectPtr>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<ObjectPtr>& refObject = ClazzObject<ReferenceObject<ObjectPtr>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const ObjectPtr>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const ObjectPtr>& refObject = ClazzObject<ReferenceObject<const ObjectPtr>>::get_val(object);
                return refObject.get_ref();
            }
            else
            {
                Py_XINCREF(object);
                new(toBuffer)ObjectPtr(object, &Deleter::Owner);
                return *reinterpret_cast<ObjectPtr *>(toBuffer);
            }
        }
        static const ObjectPtr &from_python(PyObject *object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<ObjectPtr>>::is_ref(object))
            {
                ReferenceObject<ObjectPtr>& refObject = ClazzObject<ReferenceObject<ObjectPtr>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const ObjectPtr>>::is_ref(object))
            {
                ReferenceObject<const ObjectPtr>& refObject = ClazzObject<ReferenceObject<const ObjectPtr>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const ObjectPtr& can only originates from ref const ObjectPtr type or ref ObjectPtr");
        }
        static PyObject *to_python(const ObjectPtr& value)
        {
            return ReferenceObject<const ObjectPtr>::alloc(value);
        }
    };
    
    template<>
    struct Object<PyObject*>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef PyObject* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static PyObject* get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            return *(PyObject**)fromBuffer;
        }
        static PyObject* from_python(PyObject* object)
        {
            return object;
        }
        static PyObject* to_python(PyObject*& value)
        {
            Py_XINCREF(value);
            return value;
        }
    };

    template<typename... Args>
    struct ObjectsPackSize{};

    template<>
    struct ObjectsPackSize<>
    {
        static const int value = 0;
    };

    template<typename Type, typename... Args>
    struct ObjectsPackSize<Type, Args...>
    {
        static const int value = sizeof(Type) + ObjectsPackSize<Args...>::value;
    };

    enum OffsetType : short
    {
        FromPython,
        ToNative
    };

    template<OffsetType, typename... Args>
    struct ObjectOffset{};


    template<typename T, typename... Args>
    struct ObjectOffset<FromPython, T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename... Args>
    struct ObjectOffset<ToNative, T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename X, typename... Args>
    struct ObjectOffset<FromPython, T, X, Args...>
    {
        static const int value = ObjectOffset<FromPython, T, Args...>::value + sizeof(typename X::FromPythonType);
    };

    template<typename T, typename X, typename... Args>
    struct ObjectOffset<ToNative, T, X, Args...>
    {
        static const int value = ObjectOffset<ToNative, T, Args...>::value + sizeof(typename X::Type);
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper
    {

        typedef typename Object<T>::FromPythonType FromPythonType;
        typedef typename Object<T>::Type Type;
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

    template<typename T, std::size_t I>
    struct ObjectWrapper<T&, I>
    {
        typedef typename Object<T&>::FromPythonType FromPythonType;
        typedef typename Object<T&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<const T&, I>
    {
        typedef typename Object<const T&>::FromPythonType FromPythonType;
        typedef typename Object<const T&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<T&&, I>
    {
        typedef typename Object<T&&>::FromPythonType FromPythonType;
        typedef typename Object<T&&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I, size_t N>
    struct ObjectWrapper<char(&)[N], I>
    {
        typedef typename Object<char[N]>::FromPythonType FromPythonType;
        typedef typename Object<char[N]>::Type Type;
        static void* destructor(char* buffer) { return nullptr; }
    };

    template<std::size_t I, size_t N>
    struct ObjectWrapper<const char(&)[N], I>
    {
        typedef typename Object<const char[N]>::FromPythonType FromPythonType;
        typedef typename Object<const char[N]>::Type Type;
        static void* destructor(char* buffer)
        {
            delete [] buffer;
            return nullptr;
        }
    };

    template<std::size_t I>
    struct ObjectWrapper<char*, I>
    {
        typedef typename Object<char*>::FromPythonType FromPythonType;
        typedef typename Object<char*>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const char*, I>
    {
        typedef typename Object<const char*>::FromPythonType FromPythonType;
        typedef typename Object<const char*>::Type Type;
        static void* destructor(char* buffer)
        {
            if(*reinterpret_cast<std::uint32_t*>(buffer) != MAGIC_WORD)
            {
                char* _buffer = *reinterpret_cast<char**>(buffer);
                delete [] _buffer;
            }
            return nullptr;
        }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<char const *&, I>
    {
        typedef typename Object<char const *&>::FromPythonType FromPythonType;
        typedef typename Object<char const *&>::Type Type;
        static void* destructor(char* buffer)
        {
            if(*reinterpret_cast<std::uint32_t*>(buffer) != MAGIC_WORD)
            {
                char* _buffer = *reinterpret_cast<char**>(buffer);
                delete [] _buffer;
            }
            return nullptr;
        }
    };

    template<std::size_t I>
    struct ObjectWrapper<std::string&, I>
    {
        typedef typename Object<std::string&>::FromPythonType FromPythonType;
        typedef typename Object<std::string&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const std::string&, I>
    {
        typedef typename Object<const std::string&>::FromPythonType FromPythonType;
        typedef typename Object<const std::string&>::Type Type;
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
    
    template<std::size_t I, typename X>
    struct ObjectWrapper<const std::vector<X>&, I>
    {
        typedef typename Object<const std::vector<X>&>::FromPythonType FromPythonType;
        typedef typename Object<const std::vector<X>&>::Type Type;
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

    template<std::size_t I>
    struct ObjectWrapper<double&, I>
    {
        typedef typename Object<double&>::FromPythonType FromPythonType;
        typedef typename Object<double&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const double&, I>
    {
        typedef typename Object<const double&>::FromPythonType FromPythonType;
        typedef typename Object<const double&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<int&, I>
    {
        typedef typename Object<int&>::FromPythonType FromPythonType;
        typedef typename Object<int&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const int&, I>
    {
        typedef typename Object<const int&>::FromPythonType FromPythonType;
        typedef typename Object<const int&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<ObjectPtr&, I>
    {
        typedef typename Object<ObjectPtr&>::FromPythonType FromPythonType;
        typedef typename Object<ObjectPtr&>::Type Type;
        static void* destructor(char* buffer){ return nullptr; }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<const ObjectPtr&, I>
    {
        typedef typename Object<const ObjectPtr&>::FromPythonType FromPythonType;
        typedef typename Object<const ObjectPtr&>::Type Type;
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

    template<std::size_t I>
    struct ObjectWrapper<void, I>
    {
        static void* AllocateType(Module& module) { return nullptr; }
    };
}
