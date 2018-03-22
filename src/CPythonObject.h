#pragma once

#include <type_traits>
#include <Python.h>
#include "Lock.h"

namespace pycppconn{
    template<typename T, typename std::enable_if<std::__or_<std::__not_<std::is_pointer<T>>,std::is_same<T, const char*>>::value, bool>::type = true>
    struct Object {
        typedef PyObject* Type;
        static constexpr const char* Format = "o";
        static T& GetTyped(char* data){ return *reinterpret_cast<T*>(data + sizeof(PyObject)); } //Non python types representation - PyPbject Header + Native data
    };

    template<>
    struct Object<const char*> {
    public:
        typedef const char* Type;
        static constexpr const char *Format = "z";
        static const char*& GetTyped(char* data){ return *reinterpret_cast<const char**>(data); }
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
    struct Object<bool> {
    public:
        typedef long Type;
        static constexpr const char *Format = "l";
        static bool& GetTyped(char* data){ return *reinterpret_cast<bool*>(data); }
        static bool FromPython(PyObject* obj){
            GilLock lock;
            return (bool)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(bool& data){
            GilLock lock;
            return PyBool_FromLong(data);
        }
    };

    template<>
    struct Object<int> {
    public:
        typedef long Type;
        static constexpr const char *Format = "l";
        static int& GetTyped(char* data){ return *reinterpret_cast<int*>(data); }
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

    template<typename T, typename... Args>
    struct ObjectsPackSize<T, Args...>
    {
        static const int value = sizeof(typename Object<T>::Type) + ObjectsPackSize<Args...>::value;
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
        static const int value = ObjectOffset<T, Args...>::value + sizeof(typename X::Type);
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper
    {
        typedef typename Object<T>::Type Type;
        template<typename... Args>
        static void MultiDestructors(Args&&...){}
        static void* Destructor(char* buffer){
            T* typedPtr = reinterpret_cast<T*>(buffer);
            typedPtr->~T();
            return nullptr;
        }
    };
}
