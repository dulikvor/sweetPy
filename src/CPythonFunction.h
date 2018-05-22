#pragma once

#include <initializer_list>
#include <memory>
#include <Python.h>
#include "Lock.h"
#include "Common.h"
#include "CPyModuleContainer.h"
#include "CPythonObject.h"
#include "Exception.h"
#include "ICPythonFunction.h"
#include "CPythonRefObject.h"

namespace pycppconn {

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
            std::swap(m_pyMethod,obj.m_pyMethod);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
            std::swap(m_pyMethod, obj.m_pyMethod);
        }

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::__not_<std::is_same<Return, void>>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }

            ClassType* _this;
            if(CPythonRefType<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                 _this = reinterpret_cast<ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                    ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                            typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);

            return Object<Return>::ToPython(returnValue);
        }

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::is_same<Return, void>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }

            ClassType* _this;
            if(CPythonRefType<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                                                                                                                    typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            try {
                return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc){
                exc.Raise();
                return NULL;
            }

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
    class CPythonFunction<ClassType, Return(ClassType::*)(Args...) const> : public ICPythonFunction{
    public:
        typedef Return(ClassType::*MemberFunction)(Args...) const;
        typedef CPythonFunction<ClassType, Return(ClassType::*)(Args...) const> Self;

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

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::__not_<std::is_same<Return, void>>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }

            const ClassType* _this;
            if(CPythonRefType<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                                                                                                                    typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);

            return Object<Return>::ToPython(returnValue);
        }

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::is_same<Return, void>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }

            const ClassType* _this;
            if(CPythonRefType<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                                                                                                                    typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            try {
                return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc){
                exc.Raise();
                return NULL;
            }

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

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::__not_<std::is_same<Return, void>>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(typeid(Self).hash_code()));
            Return returnValue = (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                                                                                                                    typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);
            return Object<Return>::ToPython(returnValue);
        }

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<std::is_same<Return, void>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,
                        typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value)...), "Invalid argument was provided");
            }
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(typeid(Self).hash_code()));
            (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType,typename ObjectWrapper<typename base<Args>::Type, I>::FromPythonType...>::value,
                    nativeArgsBuffer + ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value))...);

            ObjectWrapper<int, 0>::MultiDestructors(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<typename ObjectWrapper<typename base<Args>::Type, I>::Type,
                                                                                                                    typename ObjectWrapper<typename base<Args>::Type, I>::Type...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            try{
                return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc){
                exc.Raise();
                return NULL;
            }
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

