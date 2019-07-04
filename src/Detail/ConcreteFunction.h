#pragma once

#include <initializer_list>
#include <memory>
#include <utility>
#include <Python.h>
#include "Core/Lock.h"
#include "Core/Traits.h"
#include "Core/Exception.h"
#include "Core/Stack.h"
#include "Core/Assert.h"
#include "Function.h"
#include "MetaClass.h"
#include "TypesContainer.h"
#include "CPythonObject.h"
#include "ClazzPyType.h"
#include "ModuleContext.h"

namespace sweetPy {

    template<typename T, enable_if_t<std::is_copy_constructible<T>::value, bool> = true>
    static PyObject* convert_return(T& value)
    {
        return Object<T>::to_python(value);
    }

    template<typename T, enable_if_t<!std::is_copy_constructible<T>::value &&
                                     std::is_move_constructible<T>::value, bool> = true>
    static PyObject* convert_return(T& value)
    {
        return Object<T>::to_python(std::move(value));
    }

    template<typename Return, typename... Args>
    class MemberFunction: public Function
    {
        virtual ~MemberFunction() = default;
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class MemberFunction<ClassType, Return(MemberFunctionType::*)(Args...)> : public Function{
    public:
        typedef Return(MemberFunctionType::*MemberFunctionPtr)(Args...);
        typedef MemberFunction<ClassType, Return(MemberFunctionType::*)(Args...)> Self;
        typedef enable_if_t<std::is_base_of<MemberFunctionType, ClassType>::value> VirtualSupport;

        MemberFunction(const std::string& name, const std::string& doc, const MemberFunctionPtr &memberMethod)
                : Function(name, doc), m_memberMethod(memberMethod) {}

        virtual ~MemberFunction(){}
        MemberFunction(MemberFunction &) = delete;
        MemberFunction &operator=(MemberFunction &) = delete;

        MemberFunction(MemberFunction &&obj)
                : Function(std::move(static_cast<Function&>(obj))), m_memberMethod(nullptr)
        {
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        MemberFunction &operator=(MemberFunction &&obj)
        {
            Function::operator=(std::move(static_cast<Function&>(obj)));
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<!std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }

            ObjectPtr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::from_python(unicodeName.get());

            ClassType* _this;
            using RefObject = ReferenceObject<ClassType>;
            if(ClazzObject<RefObject>::is_ref(_self.get()))
            {
                auto& refObject = ClazzObject<RefObject>::get_val(_self.get());
                _this = &refObject.get_ref();
            }
            else
                _this = &ClazzObject<ClassType>::get_val(_self.get());
            
            ClazzContext& context = static_cast<ClazzPyType<ClassType>&>(
                    *reinterpret_cast<PyHeapTypeObject*>(_self->ob_type)).get_context();
            Function& function = context.get_member_function(std::hash<std::string>()(name));
            Self& m_pyFunc = static_cast<Self&>(function);
            
            Return result = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                    ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                            ObjectWrapper<Args, I>...>::value)...);

            return convert_return<Return>(result);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }

            ObjectPtr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::from_python(unicodeName.get());

            ClassType* _this;
            using RefObject = ReferenceObject<ClassType>;
            if(ClazzObject<RefObject>::is_ref(_self.get()))
            {
                auto& refObject = ClazzObject<RefObject>::get_val(_self.get());
                _this = &refObject.get_ref();
            }
            else
                _this = &ClazzObject<ClassType>::get_val(_self.get());
    
            ClazzContext& context = static_cast<ClazzPyType<ClassType>&>(
                    *reinterpret_cast<PyHeapTypeObject*>(_self->ob_type)).get_context();
            Function& function = context.get_member_function(std::hash<std::string>()(name));
            Self& m_pyFunc = static_cast<Self&>(function);

            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>, ObjectWrapper<Args, I>...>::value)...);
            Py_XINCREF(Py_None);
            return Py_None;
        }

        static PyObject* wrapper(PyObject *self, PyObject *args) {
            try
            {
                return wrapper_impl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc)
            {
                exc.raise();
                return NULL;
            }

        }

        MethodDefPtr to_python() const override
        {
            return MethodDefPtr(new PyMethodDef{
                    m_name.c_str(),
                    &wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }
        
    private:
        MemberFunctionPtr m_memberMethod;
    };

    template<typename ClassType, typename MemberFunctionType, typename Return, typename... Args>
    class MemberFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> : public Function
    {
    public:
        typedef Return(MemberFunctionType::*MemberFunctionPtr)(Args...) const;
        typedef MemberFunction<ClassType, Return(MemberFunctionType::*)(Args...) const> Self;

        MemberFunction(const std::string& name, const std::string doc, const MemberFunctionPtr &memberMethod)
                : Function(name, doc), m_memberMethod(memberMethod) {}

        virtual ~MemberFunction(){}
        MemberFunction(MemberFunction &) = delete;
        MemberFunction &operator=(MemberFunction &) = delete;

        MemberFunction(MemberFunction &&obj)
                : Function(std::move(static_cast<Function&>(obj))), m_memberMethod(nullptr)
        {
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        MemberFunction &operator=(MemberFunction &&obj)
        {
            Function::operator=(std::move(static_cast<Function&>(obj)));
            std::swap(m_memberMethod, obj.m_memberMethod);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<!std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }

            ObjectPtr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::from_python(unicodeName.get());
    
            ClassType* _this;
            using RefObject = ReferenceObject<ClassType>;
            if(ClazzObject<RefObject>::is_ref(_self.get()))
            {
                auto& refObject = ClazzObject<RefObject>::get_val(_self.get());
                _this = &refObject.get_ref();
            }
            else
                _this = &ClazzObject<ClassType>::get_val(_self.get());
    
            ClazzContext& context = static_cast<ClazzPyType<ClassType>&>(
                    *reinterpret_cast<PyHeapTypeObject*>(_self->ob_type)).get_context();
            Function& function = context.get_member_function(std::hash<std::string>()(name));
            Self& m_pyFunc = static_cast<Self&>(function);
            
            Return result = (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                        ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                            ObjectWrapper<Args, I>...>::value)...);
            return convert_return<Return>(result);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }

            ObjectPtr _self(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr unicodeName(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            std::string name = Object<std::string>::from_python(unicodeName.get());
    
            ClassType* _this;
            using RefObject = ReferenceObject<ClassType>;
            if(ClazzObject<RefObject>::is_ref(_self.get()))
            {
                auto& refObject = ClazzObject<RefObject>::get_val(_self.get());
                _this = &refObject.get_ref();
            }
            else
                _this = &ClazzObject<ClassType>::get_val(_self.get());
    
            ClazzContext& context = static_cast<ClazzPyType<ClassType>&>(
                    *reinterpret_cast<PyHeapTypeObject*>(_self->ob_type)).get_context();
            Function& function = context.get_member_function(std::hash<std::string>()(name));
            Self& m_pyFunc = static_cast<Self&>(function);
            
            (_this->*m_pyFunc.m_memberMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...);
            
            Py_XINCREF(Py_None);
            return Py_None;
        }

        static PyObject* wrapper(PyObject *self, PyObject *args) {
            try
            {
                return wrapper_impl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc)
            {
                exc.raise();
                return NULL;
            }
        }

        MethodDefPtr to_python() const override
        {
            return MethodDefPtr(new PyMethodDef{
                    m_name.c_str(),
                    &wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

    private:
        MemberFunctionPtr m_memberMethod;
    };

    template<typename Return, typename... Args>
    class StaticFunction: public Function
    {
        virtual ~StaticFunction() = default;
    };

    template<typename ClassType, typename Return, typename... Args>
    class StaticFunction<ClassType, Return(*)(Args...)> : public Function
    {
    public:
        typedef Return(*StaticFunctionPtr)(Args...);
        typedef StaticFunction<ClassType, Return(*)(Args...)> Self;

        StaticFunction(const std::string& name, const std::string doc, const StaticFunctionPtr &staticMethod)
                : Function(name, doc), m_staticMethod(staticMethod) {}

        virtual ~StaticFunction() = default;
        StaticFunction(StaticFunction &) = delete;
        StaticFunction &operator=(StaticFunction &) = delete;

        StaticFunction(StaticFunction &&obj)
            : Function(std::move(static_cast<Function&>(obj))), m_staticMethod(nullptr)
        {
            std::swap(m_staticMethod, obj.m_staticMethod);
        }

        StaticFunction &operator=(StaticFunction &&obj)
        {
            Function::operator=(std::move(static_cast<Function&>(obj)));
            std::swap(m_staticMethod, obj.m_staticMethod);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<!std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }
    
            ObjectPtr capsule(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            MetaClass& meta = *reinterpret_cast<MetaClass*>(PyCapsule_GetPointer(capsule.get(), nullptr));
            ObjectPtr hash_code(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            unsigned long typed_hash_code = PyLong_AsUnsignedLong(hash_code.get());
            
            ClazzContext& context = meta.get_context();
            Function& function = context.get_member_function(typed_hash_code);
            Self& m_pyFunc = static_cast<Self&>(function);

            Return result = (*m_pyFunc.m_staticMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                        ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                            ObjectWrapper<Args, I>...>::value)...);
            
            return convert_return<Return>(result);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }
    
            ObjectPtr capsule(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            MetaClass& meta = *reinterpret_cast<MetaClass*>(PyCapsule_GetPointer(capsule.get(), nullptr));
            ObjectPtr hash_code(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            unsigned long typed_hash_code = PyLong_AsUnsignedLong(hash_code.get());
    
            ClazzContext& context = meta.get_context();
            Function& function = context.get_member_function(typed_hash_code);
            Self& typedFunc = static_cast<Self&>(function);
            
            (*typedFunc.m_staticMethod)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                    ObjectWrapper<Args, I>...>::value)...);
            
            Py_XINCREF(Py_None);
            return Py_None;
        }

        static PyObject* wrapper(PyObject *self, PyObject *args)
        {
            try
            {
                return wrapper_impl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc)
            {
                exc.raise();
                return NULL;
            }
        }

        MethodDefPtr to_python() const override
        {
            return MethodDefPtr(new PyMethodDef{
                    m_name.c_str(),
                    &wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

    private:
        StaticFunctionPtr m_staticMethod;
    };

    template<typename Return, typename... Args>
    class CFunction: public Function
    {
        virtual ~CFunction() = default;
    };

    template<typename Return, typename... Args>
    class CFunction<Return(*)(Args...)> : public Function
    {
    public:
        typedef Return(*CFunctionPtr)(Args...);
        typedef CFunction<Return(*)(Args...)> Self;

        CFunction(const std::string& name, const std::string doc, const CFunctionPtr &function)
                : Function(name, doc), m_function(function) {}

        virtual ~CFunction() = default;

        CFunction(CFunction &) = delete;
        CFunction &operator=(CFunction &) = delete;

        CFunction(CFunction &&obj)
            : Function(std::move(static_cast<Function&>(obj))), m_function(nullptr)
        {
            std::swap(m_function, obj.m_function);
        }

        CFunction &operator=(CFunction &&obj)
        {
            Function::operator=(std::move(static_cast<Function&>(obj)));
            std::swap(m_function, obj.m_function);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<!std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }
    
            ObjectPtr contextCapsule(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr hash_code(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            unsigned long typed_hash_code = PyLong_AsUnsignedLong(hash_code.get());
    
            auto& context = *reinterpret_cast<ModuleContext*>(PyCapsule_GetPointer(contextCapsule.get(), nullptr));
            context.get_function(typed_hash_code);
    
            Function& function = context.get_function(typed_hash_code);
            Self& typedFunc = static_cast<Self&>(function);
            
            Return result = (*typedFunc.m_function)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                    ObjectWrapper<Args, I>...>::value)...);
            
            return convert_return<Return>(result);
        }

        template<bool Enable = true, std::size_t... I>
        static enable_if_t<std::is_same<Return, void>::value && Enable, PyObject*>
        wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>)
        {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }
    
            ObjectPtr contextCapsule(PyTuple_GET_ITEM(self, 0), &Deleter::Borrow);
            ObjectPtr hash_code(PyTuple_GET_ITEM(self, 1), &Deleter::Borrow);
            unsigned long typed_hash_code = PyLong_AsUnsignedLong(hash_code.get());
    
            auto& context = *reinterpret_cast<ModuleContext*>(PyCapsule_GetPointer(contextCapsule.get(), nullptr));
            context.get_function(typed_hash_code);
    
            Function& function = context.get_function(typed_hash_code);
            Self& typedFunc = static_cast<Self&>(function);
    
            (*typedFunc.m_function)(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);

            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                    ObjectWrapper<Args, I>...>::value)...);
            
            Py_XINCREF(Py_None);
            return Py_None;
        }

        static PyObject* wrapper(PyObject *self, PyObject *args)
        {
            try
            {
                return wrapper_impl(self, args, std::make_index_sequence<sizeof...(Args)>{});
            }
            catch(const CPythonException& exc)
            {
                exc.raise();
                return NULL;
            }
        }

        MethodDefPtr to_python() const override
        {
            return MethodDefPtr(new PyMethodDef{
                    m_name.c_str(),
                    &wrapper,
                    METH_VARARGS,
                    m_doc.c_str()
            });
        }

    private:
        CFunctionPtr m_function;
    };
}

