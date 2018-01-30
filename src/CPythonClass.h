#pragma once

#include <type_traits>
#include <vector>
#include <memory>
#include <Python.h>

namespace pycppconn{

    template<typename Return, typename... Args>
    class CPythonFunction
    {
    public:
        typedef Return (*MemberFunction)(Args...);
        CPythonFunction(MemberFunction memberFunction):m_memberFunction(memberFunction){}
        CPythonFunction(CPythonFunction&) = delete;
        CPythonFunction& operator=(CPythonFunction&) = delete;
        CPythonFunction(CPythonFunction&& obj):m_memberFunction(nullptr){
            std::swap(m_memberFunction, obj.m_memberFunction);
        }
        CPythonFunction& operator=(CPythonFunction&& obj){
            std::swap(m_memberFunction, obj.m_memberFunction);
        }
    private:
        MemberFunction m_memberFunction;
    };


    template<typename T, typename Type = typename std::remove_const<typename std::remove_pointer<
            typename std::remove_reference<T>::type>::type>::type>
    class CPythonClass {
    public:
        template<typename Return, typename... Args>
        void AddMethod(CPythonFunction<Return, Args...>&& memberFunction){}
        std::unique_ptr<PyTypeObject> ToPython() const;
    private:
        std::vector<PyMethodDef*> m_methods;
    };
}
