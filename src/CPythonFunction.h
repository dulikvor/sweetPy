#pragma once

#include <initializer_list>
#include <memory>
#include <Python.h>
#include "Lock.h"
#include "Common.h"
#include "CPyModuleContainer.h"

namespace pycppconn {


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
    struct ArgumentsPackCount{};

    template<>
    struct ArgumentsPackCount<>
    {
        static const int value = 0;
    };

    template<typename T, typename... Args>
    struct ArgumentsPackCount<T, Args...>
    {
        static const int value = 1 + ArgumentsPackCount<Args...>::value;
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

    class ICPythonFunction
    {
    public:
        virtual std::unique_ptr<PyMethodDef> ToPython() const = 0;
    };


    template<typename Return, typename... Args>
    class CPythonFunction: public ICPythonFunction {
    };

    template<typename ClassType, typename Return, typename... Args>
    class CPythonFunction<ClassType, Return(ClassType::*)(Args...)> : public ICPythonFunction{
    public:
        typedef Return(ClassType::*MemberFunction)(Args...);
        typedef CPythonFunction<ClassType, Return(ClassType::*)(Args...)> Self;

        CPythonFunction(const std::string& name, const std::string doc, const MemberFunction &memberFunction)
                : m_name(name), m_doc(doc), m_memberFunction(memberFunction) {
        }

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_memberFunction(nullptr) {
            std::swap(m_memberFunction, obj.m_memberFunction);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
            std::swap(m_pyMethod, obj.m_pyMethod);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberFunction, obj.m_memberFunction);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
            std::swap(m_pyMethod, obj.m_pyMethod);
        }

        template<std::size_t... I>
        static PyObject* WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Argument<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char buffer[ArgumentsPackSize<typename base<Args>::Type...>::value];
            {
                GilLock lock;
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (buffer + ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,ArgumentWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }
            ClassType* _this = reinterpret_cast<ClassType*>(self);
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberFunction)(std::forward<Args>(Argument<typename base<Args>::Type>::ToNative(
                    buffer + ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,ArgumentWrapper<typename base<Args>::Type, I>...>::value))...);
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            WrapperImpl(self, args, std::make_index_sequence<ArgumentsPackCount<Args...>::value>{});
        }

        std::unique_ptr<PyMethodDef> ToPython() const override{
            return std::unique_ptr<PyMethodDef>(new PyMethodDef{
                    m_name.c_str(),
                    &Wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

    private:
        std::unique_ptr<PyMethodDef> m_pyMethod;
        MemberFunction m_memberFunction;
        std::string m_name;
        std::string m_doc;
    };
}

