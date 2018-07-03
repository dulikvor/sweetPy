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
#include "CPythonRef.h"

namespace sweetPy {

    template<typename Return, typename... Args>
    class CPythonFunction: public ICPythonFunction {
        virtual ~CPythonFunction(){}
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class CPythonFunction<ClassType, Return(MemberFunctionType::*)(Args...)> : public ICPythonFunction{
    public:
        typedef Return(MemberFunctionType::*MemberFunction)(Args...);
        typedef CPythonFunction<ClassType, Return(MemberFunctionType::*)(Args...)> Self;
        typedef typename std::enable_if<std::is_base_of<MemberFunctionType, ClassType>::value>::type VirtualSupport;

        CPythonFunction(const std::string& name, const std::string doc, const MemberFunction &memberMethod)
                : m_name(name), m_doc(doc), m_memberMethod(memberMethod) {
        }

        virtual ~CPythonFunction(){}

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_memberMethod(nullptr) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                 _this = reinterpret_cast<ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                    ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                            ObjectWrapper<typename base<Args>::Type, I>...>::value)...);

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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            try
            {
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

        void AllocateObjectsTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateObjectType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateObjectType(module);
        }

    private:
        MemberFunction m_memberMethod;
        std::string m_name;
        std::string m_doc;
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class CPythonFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> : public ICPythonFunction{
    public:
        typedef Return(MemberFunctionType::*MemberFunction)(Args...) const;
        typedef CPythonFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> Self;

        CPythonFunction(const std::string& name, const std::string doc, const MemberFunction &memberMethod)
                : m_name(name), m_doc(doc), m_memberMethod(memberMethod) {
        }

        virtual ~CPythonFunction(){}

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_memberMethod(nullptr) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_memberMethod, obj.m_memberMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            const ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            const ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(self))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(self + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(self + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(typeid(Self).hash_code()));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
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

        void AllocateObjectsTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateObjectType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateObjectType(module);
        }

    private:
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

        virtual ~CPythonFunction(){}

        CPythonFunction(CPythonFunction &) = delete;

        CPythonFunction &operator=(CPythonFunction &) = delete;

        CPythonFunction(CPythonFunction &&obj) : m_staticMethod(nullptr) {
            std::swap(m_staticMethod, obj.m_staticMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
        }

        CPythonFunction &operator=(CPythonFunction &&obj) {
            std::swap(m_staticMethod, obj.m_staticMethod);
            std::swap(m_name, obj.m_name);
            std::swap(m_doc, obj.m_doc);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(typeid(Self).hash_code()));
            Return returnValue = (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(typeid(Self).hash_code()));
            (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + FromPythonObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
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

        void AllocateObjectsTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateObjectType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateObjectType(module);
        }

    private:
        StaticFunction m_staticMethod;
        std::string m_name;
        std::string m_doc;
    };
}

