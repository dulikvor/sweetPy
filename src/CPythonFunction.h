#pragma once

#include <initializer_list>
#include <memory>
#include <Python.h>
#include "Lock.h"
#include "Common.h"

namespace pycppconn {

    template<typename T>
    struct ArgumentConvertor {
    };

    template<>
    struct ArgumentConvertor<std::string> {
    public:
        static constexpr const char *Format = "z";
        thread_local static const char* Data;

    };

    template<>
    struct ArgumentConvertor<int> {
    public:
        static constexpr const char *Format = "d";
        thread_local static int Data;
    };

    class ICPythonFunction
    {
    public:
        virtual PyMethodDef* ToPython() = 0;
    };


    template<typename Return, typename... Args>
    class CPythonFunction: public ICPythonFunction {
    };

    template<typename Return, typename... Args>
    class CPythonFunction<Return(*)(Args...)> : public ICPythonFunction{
    public:
        typedef Return (*MemberFunction)(Args...);

        CPythonFunction(const std::string& name, const std::string doc, const MemberFunction &memberFunction)
                : m_name(name), m_doc(doc), m_memberFunction(memberFunction) {
            m_pyMethod.reset(new PyMethodDef{m_name.c_str(), reinterpret_cast<PyCFunction>(this), METH_VARARGS, m_doc.c_str()});
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

        PyObject* operator()(PyObject *self, PyObject *args) {
            std::initializer_list<const char *> formatList = {ArgumentConvertor<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            {
                GilLock lock;
                CPYTHON_VERIFY(!PyArg_ParseTuple(args, format.c_str(), &ArgumentConvertor<Args>::Data...), "Invalid argument was provided");
            }
            (*m_memberFunction)(ArgumentConvertor<Args>::Data...);
        }

        PyMethodDef* ToPython() override{
            return m_pyMethod.get();
        }

    private:
        std::unique_ptr<PyMethodDef> m_pyMethod;
        MemberFunction m_memberFunction;
        std::string m_name;
        std::string m_doc;
    };
}

