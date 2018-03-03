#pragma once

#include <Python.h>

namespace pycppconn{
    template<typename T>
    struct Argument {
        typedef PyObject* Type;
        static constexpr const char* Format = "o";
        static T& ToNative(char* data){ return *reinterpret_cast<T*>(data + sizeof(PyObject)); } //Non python types representation - PyPbject Header + Native data
    };

    template<>
    struct Argument<const char*> {
    public:
        typedef const char* Type;
        static constexpr const char *Format = "z";
        static const char*& ToNative(char* data){ return *reinterpret_cast<const char**>(data); }
    };

    template<>
    struct Argument<int> {
    public:
        typedef int Type;
        static constexpr const char *Format = "i";
        static int& ToNative(char* data){ return *reinterpret_cast<int*>(data); }
    };

    template<typename... Args>
    struct ArgumentsPackSize{};

    template<>
    struct ArgumentsPackSize<>
    {
        static const int value = 0;
    };

    template<typename T, typename... Args>
    struct ArgumentsPackSize<T, Args...>
    {
        static const int value = sizeof(typename Argument<T>::Type) + ArgumentsPackSize<Args...>::value;
    };

    template<typename... Args>
    struct ArgumentOffset{};


    template<typename T, typename... Args>
    struct ArgumentOffset<T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename X, typename... Args>
    struct ArgumentOffset<T, X, Args...>
    {
        static const int value = ArgumentOffset<T, Args...>::value + sizeof(typename X::Type);
    };

    template<typename T, std::size_t I>
    struct ArgumentWrapper
    {
        typedef typename Argument<T>::Type Type;
    };
}
