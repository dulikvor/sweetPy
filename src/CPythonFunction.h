#pragma once

#include <initializer_list>
#include <memory>
#include <Python.h>

namespace pycppconn {

    template<typename T>
    struct ArgumentConvertor {
    };

    template<>
    struct ArgumentConvertor<std::string> {
    public:
        static constexpr char *Format = "z";
        thread_local static const char* Data;
    };

    template<>
    struct ArgumentConvertor<int> {
    public:
        static constexpr char *Format = "d";
        thread_local static int Data;
    };


    template<typename Return, typename... Args>
    class CPythonFunction {
    };

    template<typename Return, typename... Args>
    class CPythonFunction<Return(*)(Args...)> {
    public:
        typedef Return (*MemberFunction)(Args...);

        CPythonFunction(const MemberFunction &memberFunction) : m_memberFunction(memberFunction) {}

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_memberFunction(nullptr) {
            std::swap(m_memberFunction, obj.m_memberFunction);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberFunction, obj.m_memberFunction);
        }

        void Wrapper(PyObject *self, PyObject *args) {
            std::initializer_list<const char *> formatList = {ArgumentConvertor<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            PyArg_ParseTuple(args, format.c_str(), &ArgumentConvertor<Args>::Data...);
            (*m_memberFunction)(ArgumentConvertor<Args>::Data...);
        }


        std::unique_ptr<PyMethodDef> ToPython();

    private:
        MemberFunction m_memberFunction;
    };
}

