#pragma once

#include <initializer_list>
#include <memory>
#include <utility>
#include <Python.h>
#include "src/Core/Lock.h"
#include "src/Core/Traits.h"
#include "src/Core/Exception.h"
#include "src/Core/Stack.h"
#include "src/Core/Dictionary.h"
#include "CPyModuleContainer.h"
#include "CPythonObject.h"
#include "CPythonFunction.h"
#include "CPythonRef.h"

namespace sweetPy {

    template<typename Return, typename... Args>
    class CPythonMemberFunction: public CPythonFunction {
        virtual ~CPythonMemberFunction(){}
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class CPythonMemberFunction<ClassType, Return(MemberFunctionType::*)(Args...)> : public CPythonFunction{
    public:
        typedef Return(MemberFunctionType::*MemberFunction)(Args...);
        typedef CPythonMemberFunction<ClassType, Return(MemberFunctionType::*)(Args...)> Self;
        typedef typename std::enable_if<std::is_base_of<MemberFunctionType, ClassType>::value>::type VirtualSupport;

        CPythonMemberFunction(const std::string& name, const std::string& doc, const MemberFunction &memberMethod)
                : CPythonFunction(name, doc), m_memberMethod(memberMethod) {}

        virtual ~CPythonMemberFunction(){}
        CPythonMemberFunction(CPythonMemberFunction &) = delete;
        CPythonMemberFunction &operator=(CPythonMemberFunction &) = delete;

        CPythonMemberFunction(CPythonMemberFunction &&obj)
                : CPythonFunction(std::move(static_cast<CPythonFunction&>(obj))), m_memberMethod(nullptr)
        {
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        CPythonMemberFunction &operator=(CPythonMemberFunction &&obj)
        {
            CPythonFunction::operator=(std::move(static_cast<CPythonFunction&>(obj)));
            std::swap(m_memberMethod, obj.m_memberMethod);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            object_ptr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            object_ptr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::FromPython(unicodeName.get());

            ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(_self.get()))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(_self.get() + 1)->GetRef();
            else
                 _this = reinterpret_cast<ClassType*>(_self.get() + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(GenerateFunctionId<Self>(name)));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                    ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            object_ptr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            object_ptr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::FromPython(unicodeName.get());

            ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(_self.get()))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(_self.get() + 1)->GetRef();
            else
                _this = reinterpret_cast<ClassType*>(_self.get() + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(GenerateFunctionId<Self>(name)));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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

        void AllocateTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateType(module);
        }

    private:
        MemberFunction m_memberMethod;
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class CPythonMemberFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> : public CPythonFunction{
    public:
        typedef Return(MemberFunctionType::*MemberFunction)(Args...) const;
        typedef CPythonMemberFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> Self;

        CPythonMemberFunction(const std::string& name, const std::string doc, const MemberFunction &memberMethod)
                : CPythonFunction(name, doc), m_memberMethod(memberMethod) {}

        virtual ~CPythonMemberFunction(){}
        CPythonMemberFunction(CPythonMemberFunction &) = delete;
        CPythonMemberFunction &operator=(CPythonMemberFunction &) = delete;

        CPythonMemberFunction(CPythonMemberFunction &&obj)
                : CPythonFunction(std::move(static_cast<CPythonFunction&>(obj))), m_memberMethod(nullptr)
        {
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        CPythonMemberFunction &operator=(CPythonMemberFunction &&obj)
        {
            CPythonFunction::operator=(std::move(static_cast<CPythonFunction&>(obj)));
            std::swap(m_memberMethod, obj.m_memberMethod);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            object_ptr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            object_ptr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::FromPython(unicodeName.get());

            const ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(_self.get()))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(_self.get() + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(_self.get() + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(GenerateFunctionId<Self>(name)));
            Return returnValue = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            object_ptr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            object_ptr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::FromPython(unicodeName.get());

            const ClassType* _this;
            if(CPythonRef<>::IsReferenceType<ClassType>(_self.get()))
                _this = &reinterpret_cast<CPythonRefObject<ClassType>*>(_self.get() + 1)->GetRef();
            else
                _this = reinterpret_cast<const ClassType*>(_self.get() + 1);

            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetMethod(GenerateFunctionId<Self>(name)));
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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

        void AllocateTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateType(module);
        }

    private:
        MemberFunction m_memberMethod;
    };

    template<typename Return, typename... Args>
    class CPythonStaticFunction: public CPythonFunction {
        virtual ~CPythonStaticFunction(){}
    };

    template<typename ClassType, typename Return, typename... Args>
    class CPythonStaticFunction<ClassType, Return(*)(Args...)> : public CPythonFunction{
    public:
        typedef Return(*StaticFunction)(Args...);
        typedef CPythonStaticFunction<ClassType, Return(*)(Args...)> Self;

        CPythonStaticFunction(const std::string& name, const std::string doc, const StaticFunction &staticMethod)
                : CPythonFunction(name, doc), m_staticMethod(staticMethod) {}

        virtual ~CPythonStaticFunction(){}
        CPythonStaticFunction(CPythonStaticFunction &) = delete;
        CPythonStaticFunction &operator=(CPythonStaticFunction &) = delete;

        CPythonStaticFunction(CPythonStaticFunction &&obj)
        : CPythonFunction(std::move(static_cast<CPythonFunction&>(obj))), m_staticMethod(nullptr)
        {
            std::swap(m_staticMethod, obj.m_staticMethod);
        }

        CPythonStaticFunction &operator=(CPythonStaticFunction &&obj)
        {
            CPythonFunction::operator=(std::move(static_cast<CPythonFunction&>(obj)));
            std::swap(m_staticMethod, obj.m_staticMethod);
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            std::string name = Object<std::string>::FromPython(self);
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(GenerateFunctionId<Self>(name)));
            Return returnValue = (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            std::string name = Object<std::string>::FromPython(self);
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetStaticMethod(GenerateFunctionId<Self>(name)));
            (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                            ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args)
        {
            try
            {
                return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc){
                exc.Raise();
                return NULL;
            }
        }

        std::unique_ptr<PyMethodDef> ToPython() const override
        {
            return std::unique_ptr<PyMethodDef>(new PyMethodDef{
                    m_name.c_str(),
                    &Wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

        void AllocateTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateType(module);
        }

    private:
        StaticFunction m_staticMethod;
    };

    template<typename Return, typename... Args>
    class CPythonCFunction: public CPythonFunction {
        virtual ~CPythonCFunction(){}
    };

    template<typename Return, typename... Args>
    class CPythonCFunction<Return(*)(Args...)> : public CPythonFunction{
    public:
        typedef Return(*Function)(Args...);
        typedef CPythonCFunction<Return(*)(Args...)> Self;

        CPythonCFunction(const std::string& name, const std::string doc, const Function &function)
                : CPythonFunction(name, doc), m_function(function) {}

        virtual ~CPythonCFunction(){}

        CPythonCFunction(CPythonCFunction &) = delete;
        CPythonCFunction &operator=(CPythonCFunction &) = delete;

        CPythonCFunction(CPythonCFunction &&obj)
        : CPythonFunction(std::move(static_cast<CPythonFunction&>(obj))), m_function(nullptr)
        {
            std::swap(m_function, obj.m_function);
        }

        CPythonCFunction &operator=(CPythonCFunction &&obj)
        {
            CPythonFunction::operator=(std::move(static_cast<CPythonFunction&>(obj)));
            std::swap(m_function, obj.m_function);
        }

        template<bool Enable = true, std::size_t... I>
        static typename std::enable_if<!std::is_same<Return, void>::value && Enable, PyObject*>::type WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            std::string name = Object<std::string>::FromPython(self);
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetGlobalFunction(GenerateFunctionId<Self>(name)));
            Return returnValue = (*m_pyFunc.m_function)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                        ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
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
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }

            std::string name = Object<std::string>::FromPython(self);
            Self& m_pyFunc = static_cast<Self&>(CPyModuleContainer::Instance().GetGlobalFunction(GenerateFunctionId<Self>(name)));
            (*m_pyFunc.m_function)(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);

            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                        ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
            return Py_None;
        }

        static PyObject* Wrapper(PyObject *self, PyObject *args) {
            try
            {
                return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc)
            {
                exc.Raise();
                return NULL;
            }
        }

        std::unique_ptr<PyMethodDef> ToPython() const override
        {
            return std::unique_ptr<PyMethodDef>(new PyMethodDef{
                    m_name.c_str(),
                    &Wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

        void AllocateTypes(CPythonModule& module) const override
        {
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, 0>::AllocateType(module)...);
            ObjectWrapper<typename base<Return>::Type, 0>::AllocateType(module);
        }

    private:
        Function m_function;
    };
}

