#pragma once

#include <string>
#include <type_traits>
#include <Python.h>
#include "Lock.h"
#include "CPythonRefObject.h"
#include "Exception.h"

namespace pycppconn{
    template<typename T, typename std::enable_if<std::__or_<std::__not_<std::is_pointer<T>>,std::is_same<T, const char*>>::value, bool>::type = true>
    struct Object {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static T& GetTyped(char* data){ return *reinterpret_cast<T*>(data + sizeof(PyObject)); } //Non python types representation - PyPbject Header + Native data
    };

    template<typename T>
    struct Object<T&>{
    public:
        typedef PyObject* FromPythonType;
        typedef typename std::enable_if<std::__not_<std::is_same<T&, std::string&>>::value, T>::type Type;
        static constexpr const char *Format = "O";
        static T& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* obj = *reinterpret_cast<PyObject**>(fromBuffer);
            if(obj->ob_type == &CPythonRefType::GetType()){
                CPythonRefObject<T&>* refObject = reinterpret_cast<CPythonRefObject<T&>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                return *reinterpret_cast<T*>(obj + 1);
            }
        }
        static T& FromPython(PyObject* obj){
            if(obj->ob_type == &CPythonRefType::GetType()){
                CPythonRefObject<T&>* refObject = reinterpret_cast<CPythonRefObject<T&>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                return *reinterpret_cast<T*>(obj + 1);
            }
        }
        static PyObject* ToPython(T& instance){
            return CPythonRefType::Alloc(instance);
        }
    };

    //All python value category are basicaly lvalue, so as long as the C++ function prototype will use reference collapsing it will receive it as lvalue.
    template<typename T>
    struct Object<T&&>{
    public:
        typedef PyObject* FromPythonType;
        typedef typename std::enable_if<std::__not_<std::is_same<T&&, std::string&&>>::value, T>::type Type;
        static constexpr const char *Format = "O";
        static T&& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* obj = *reinterpret_cast<PyObject**>(fromBuffer);
            if(obj->ob_type == &CPythonRefType::GetType()){
                CPythonRefObject<T&>* refObject = reinterpret_cast<CPythonRefObject<T&>*>(obj + 1);
                return std::move(refObject->GetRef());
            }
            else{
                return std::move(*reinterpret_cast<T*>(obj + 1));
            }
        }
        static T&& FromPython(PyObject* obj){
            if(obj->ob_type == &CPythonRefType::GetType()){
                CPythonRefObject<T&>* refObject = reinterpret_cast<CPythonRefObject<T&>*>(obj + 1);
                return std::move(refObject->GetRef());
            }
            else{
                return std::move(*reinterpret_cast<T*>(obj + 1));
            }
        }
        //No meaning to return rvalue reference to python (only supports lvalue value category), so ToPython is not implemented.
    };

    template<>
    struct Object<const char*> {
    public:
        typedef const char* FromPythonType;
        typedef const char* Type;
        static constexpr const char *Format = "z";
        static const char*& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)const char*(*reinterpret_cast<char**>(fromBuffer));
            return *reinterpret_cast<const char**>(toBuffer);
        }
        static const char* FromPython(PyObject* obj){
            GilLock lock;
            return PyString_AsString(obj);
        }
        static PyObject* ToPython(const char* data){
            GilLock lock;
            return PyString_FromString(data);
        }
    };

    template<>
    struct Object<std::string> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "z";
        static std::string& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)std::string(*reinterpret_cast<char**>(fromBuffer));
            return *reinterpret_cast<std::string*>(toBuffer);
        }
        static std::string FromPython(PyObject* obj){
            GilLock lock;
            char* data = PyString_AsString(obj); // Providing its underline buffer, a copy is in need.
            return std::string(data);
        }
        static PyObject* ToPython(const std::string& data){
            GilLock lock;
            return PyString_FromString(data.c_str());
        }
    };
    //Providing lvalue string is not possible, due to transform between two types, so FromPython is removed.
    template<>
    struct Object<std::string&> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "z";
        static std::string& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)std::string(*reinterpret_cast<char**>(fromBuffer));
            return *reinterpret_cast<std::string*>(toBuffer);
        }
        static std::string& FromPython(PyObject* obj){
            if(obj->ob_type == &CPythonRefType::GetType()){
                CPythonRefObject<std::string&>* refObject = reinterpret_cast<CPythonRefObject<std::string&>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                throw CPythonException(PyExc_TypeError, SOURCE, "conversion between python string to string& is not possible");
            }
        }
        static PyObject* ToPython(std::string& data){
            return CPythonRefType::Alloc(data);
        }
    };

    template<>
    struct Object<bool> {
    public:
        typedef long FromPythonType;
        typedef bool Type;
        static constexpr const char *Format = "l";
        static bool& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)bool(*reinterpret_cast<bool*>(fromBuffer));
            return *reinterpret_cast<bool*>(toBuffer);
        }
        static bool FromPython(PyObject* obj){
            GilLock lock;
            return (bool)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(const bool& data){
            GilLock lock;
            return PyBool_FromLong(data);
        }
    };

    template<>
    struct Object<int> {
    public:
        typedef long FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "l";
        static int& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)int(*reinterpret_cast<int*>(fromBuffer));
            return *reinterpret_cast<int*>(toBuffer);
        }
        static int FromPython(PyObject* obj){
            GilLock lock;
            return (int)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(const int& data){
            GilLock lock;
            return PyInt_FromLong(data);
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

    template<typename... Args>
    struct ObjectOffset{};


    template<typename T, typename... Args>
    struct ObjectOffset<T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename X, typename... Args>
    struct ObjectOffset<T, X, Args...>
    {
        static const int value = ObjectOffset<T, Args...>::value + sizeof(X);
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper
    {

        typedef typename Object<T>::FromPythonType FromPythonType;
        typedef typename Object<T>::Type Type;
        template<typename... Args>
        static void MultiDestructors(Args&&...){}
        static void* Destructor(char* buffer){
            Type* typedPtr = reinterpret_cast<Type*>(buffer);
            typedPtr->~Type();
            return nullptr;
        }
    };
}
