#pragma once

#include <initializer_list>
#include <memory>
#include <Python.h>
#include "Lock.h"
#include "Common.h"
#include "CPyModuleContainer.h"
#include "CPythonArgument.h"

namespace pycppconn {

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

        CPythonFunction(const std::string& name, const std::string doc, const MemberFunction &memberMethod)
                : m_name(name), m_doc(doc), m_memberMethod(memberMethod) {
        }

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_memberMethod(nullptr) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
            std::swap(m_pyMethod, obj.m_pyMethod);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberMethod, obj.m_memberMethod);
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
            ClassType* _this = reinterpret_cast<ClassType*>((char*)self + sizeof(PyObject));
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Argument<typename base<Args>::Type>::GetTyped(
                    buffer + ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,ArgumentWrapper<typename base<Args>::Type, I>...>::value))...);

            ArgumentWrapper<int, 0>::MultiDestructors(ArgumentWrapper<typename base<Args>::Type, I>::Destructor(buffer +
                    ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,
                            ArgumentWrapper<typename base<Args>::Type, I>...>::value)...);
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
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
        MemberFunction m_memberMethod;
        std::string m_name;
        std::string m_doc;
    };

    template<typename ClassType, typename Return, typename... Args>
    class CPythonFunction<ClassType, Return(*)(Args...)> : public ICPythonFunction{
    public:
        typedef Return(*StaticFunction)(Args...);
        typedef CPythonFunction<ClassType, Return(*)(Args...)> Self;

        CPythonFunction(const std::string& name, const std::string doc, const StaticFunction &staticMethod)
                : m_name(name), m_doc(doc), m_staticMethod(staticMethod) {
        }

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_staticMethod(nullptr) {
            std::swap(m_staticMethod, obj.m_staticMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
            std::swap(m_pyMethod, obj.m_pyMethod);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_staticMethod, obj.m_staticMethod);
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
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(typeid(Self).hash_code()));
            (*m_pyFunc.m_staticMethod)(std::forward<Args>(Argument<typename base<Args>::Type>::GetTyped(
                    buffer + ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,ArgumentWrapper<typename base<Args>::Type, I>...>::value))...);

            ArgumentWrapper<int, 0>::MultiDestructors(ArgumentWrapper<typename base<Args>::Type, I>::Destructor(buffer +
                                                                                                                  ArgumentOffset<ArgumentWrapper<typename base<Args>::Type, I>,
                                                                                                                          ArgumentWrapper<typename base<Args>::Type, I>...>::value)...);
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
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
        StaticFunction m_staticMethod;
        std::string m_name;
        std::string m_doc;
    };
}

