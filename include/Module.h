#pragma once

#include <Python.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "Core/Deleter.h"
#include "Core/Traits.h"
#include "Core/Exception.h"
#include "Core/PythonAssist.h"
#include "Core/Utils.h"
#include "Types/ObjectPtr.h"
#include "Types/Dictionary.h"
#include "Detail/CPythonType.h"
#include "Detail/CPythonObject.h"
#include "Detail/Function.h"
#include "Detail/ConcreteFunction.h"
#include "Detail/ModuleContext.h"
#include "Detail/PlainType.h"
#include "Detail/Object.h"

namespace sweetPy {
    class MetaClass;
    extern MetaClass* commonMetaTypePtr;
    class Variable {
    public:
        typedef std::unique_ptr<Variable> VariablePtr;
        Variable(const std::string &name) : m_name(name) {}
        virtual ~Variable() = default;
        virtual ObjectPtr to_python() const = 0;
        const std::string &get_name() const { return m_name; }
    
    private:
        std::string m_name;
    };
    
    template<typename T, typename _T = decay_t<T>>
    class TypedVariable: public Variable
    {
    public:
        TypedVariable(const std::string &name, const T& value)
                : Variable(name), m_value(value) {}
        ObjectPtr to_python() const override
        {
            return ObjectPtr(Object<_T>::to_python(m_value), &Deleter::Owner);
        }
    
    private:
        _T m_value;
    };
    
    class Module
    {
    private:
        typedef std::unique_ptr<Function> FunctionPtr;
        typedef std::size_t TypeKey;
    
        template<typename...>
        struct TypesInitializer{};
        template<typename... Args>
        struct TypesInitializer<std::tuple<Args...>>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                invoker(initialize_type<Args>(module, namePrefix)...);
            }
            template<typename T>
            static void* initialize_type(Module& module, const std::string& namePrefix)
            {
                auto objectDestroyer = [](PyObject* object){
                    T* instance = &ClazzObject<T>::get_val(object);
                    instance->~T();
                };
                std::size_t hashCode = Hash::generate_hash_code<T>();
                
                std::unique_ptr<PyObject, std::function<void(PyObject*)>> plainType(
                        CPythonType::get_py_object(
                                new PlainType(namePrefix + std::to_string(hashCode), "", ClazzObject<T>::get_size(),
                                              hashCode, objectDestroyer)
                        ),
                        [](PyObject* ptr){
                            auto plainType = static_cast<PlainType*>(CPythonType::get_type(ptr));
                            if(plainType->is_finalized() == false)
                                delete plainType;
                            else
                            {
                                auto type = reinterpret_cast<PyTypeObject*>(ptr);
                                Dictionary dict(type->tp_dict);
                                dict.clear();
                                
                                Py_XDECREF(type->tp_mro);
                                type->tp_mro = nullptr;
                                free((void*)type->tp_doc);
                                type->tp_doc = nullptr;
                                type->ob_base.ob_base.ob_refcnt -= 2;
                                
                                PyTypeObject* meta = ptr->ob_type;
                                meta->tp_dealloc(ptr);
                            }
                        }
                );
    
                CPythonType& type = *CPythonType::get_type(plainType.get());
                if(!module.is_type_exists(type.get_hash_code()))
                {
                    static_cast<PlainType&>(type).finalize();
                    TypesContainer::instance().add_type(type.get_hash_code(), type, false);
                    module.add_type(type.get_hash_code(), std::move(plainType), false);
                }
                return nullptr;
            }
        };
    
        template<typename...>
        struct ReferenceTypesInitializer{};
        template<typename... Args>
        struct ReferenceTypesInitializer<std::tuple<Args...>>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                invoker(initialize_type<Args>(module, namePrefix)...);
            }
            template<typename T, typename _T = remove_reference_t<T>>
            static void* initialize_type(Module& module, const std::string& namePrefix)
            {
                std::size_t hashCode = Hash::generate_hash_code<ReferenceObject<_T>>();
                std::unique_ptr<PyObject, std::function<void(PyObject*)>> plainType(
                        CPythonType::get_py_object(
                            new PlainType(namePrefix + std::to_string(hashCode), "", ClazzObject<ReferenceObject<_T>>::get_size(),
                                hashCode, [](PyObject*){})
                        ),
                        [](PyObject* ptr){
                            auto plainType = static_cast<PlainType*>(CPythonType::get_type(ptr));
                            if(plainType->is_finalized() == false)
                                delete plainType;
                            else
                            {
                                auto type = reinterpret_cast<PyTypeObject*>(ptr);
                                Dictionary dict(type->tp_dict);
                                dict.clear();
    
                                Py_XDECREF(type->tp_mro);
                                type->tp_mro = nullptr;
                                free((void*)type->tp_doc);
                                type->tp_doc = nullptr;
                                type->ob_base.ob_base.ob_refcnt -= 2;
                                
                                PyTypeObject* meta = ptr->ob_type;
                                meta->tp_dealloc(ptr);
                            }
                        }
                );
    
                CPythonType& type = *CPythonType::get_type(plainType.get());
                if(!module.is_type_exists(type.get_hash_code()))
                {
                    static_cast<PlainType&>(type).finalize();
                    TypesContainer::instance().add_type(type.get_hash_code(), type, false);
                    module.add_type(type.get_hash_code(), std::move(plainType), false);
                }
                return nullptr;
            }
        };
    
        template<typename Return, typename... Args>
        struct FunctionTypesInitializerImpl
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                using others = typename filter<Predicator<is_reference_predicator<>, false>, Args...>::type;
                using byReference = typename filter<Predicator<is_reference_predicator<>>, Args...>::type;
                TypesInitializer<others>::initialize_types(module, namePrefix);
                ReferenceTypesInitializer<byReference>::initialize_types(module, namePrefix + "Reference");
                initialize_return(module, namePrefix);
            }
            template<typename _Return = Return, enable_if_t<std::is_same<_Return, void>::value, bool> = true>
            static void initialize_return(Module& module, const std::string& namePrefix)
            {
            }
            template<typename _Return = Return, enable_if_t<!std::is_same<_Return, void>::value, bool> = true>
            static void initialize_return(Module& module, const std::string& namePrefix)
            {
                using others = typename filter<Predicator<is_reference_predicator<>, false>, _Return>::type;
                using byReference = typename filter<Predicator<is_reference_predicator<>>, _Return>::type;
                TypesInitializer<others>::initialize_types(module, namePrefix);
                ReferenceTypesInitializer<byReference>::initialize_types(module, namePrefix + "Reference");
            }
        };
    
        template<typename... Args>
        struct FunctionTypesInitializer{};
        template<typename Type, typename Return, typename... Args>
        struct FunctionTypesInitializer<Return(Type::*)(Args...)>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                FunctionTypesInitializerImpl<Return, Args...>::initialize_types(module, namePrefix);
            }
        };
        template<typename Type, typename Return, typename... Args>
        struct FunctionTypesInitializer<Return(Type::*)(Args...) const>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                FunctionTypesInitializerImpl<Return, Args...>::initialize_types(module, namePrefix);
            }
        };
        template<typename Return, typename... Args>
        struct FunctionTypesInitializer<Return(*)(Args...)>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                FunctionTypesInitializerImpl<Return, Args...>::initialize_types(module, namePrefix);
            }
        };
    public:
        explicit Module(const std::string &name, const std::string &doc)
            :m_moduleDef(nullptr), m_module(nullptr, &Deleter::Borrow),
             m_context(new ModuleContext()), m_name(name), m_doc(doc)
         {
            auto moduleDef = (PyModuleDef*)malloc(sizeof(PyModuleDef));
            new(moduleDef)PyModuleDef{};
            m_moduleDef.reset(moduleDef);
         }
        ~Module() = default;
        void add_type(TypeKey key, ObjectPtr&& type, bool isForceInsertion = true)
        {
            if(isForceInsertion)
                m_types.erase(key);
            
            m_types.insert(std::make_pair(key, std::move(type)));
        }
        bool is_type_exists(TypeKey key) const
        {
            return m_types.find(key) != m_types.end();
        }
        template<typename T>
        void add_variable(const std::string& name, T&& value)
        {
            m_variables.emplace_back(new TypedVariable<T>(name, std::forward<T>(value)));
        }
        template<typename X, typename = enable_if_t<std::is_function<typename std::remove_pointer<X>::type>::value>>
        void add_function(const std::string &name, const std::string &doc, X &&function)
        {
            typedef CFunction<X> CPyFuncType;
            FunctionPtr functionPtr(new CPyFuncType(name, doc, function));
            auto pair = m_functions.insert(std::make_pair(functionPtr->get_hash_code(), std::move(functionPtr)));
            FunctionTypesInitializer<X>::initialize_types(*this, "Clazz");
            if(!pair.second)
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "function key already in use");
        }
        Function& get_function(std::size_t hash_code)
        {
            auto it = m_functions.find(hash_code);
            if(it == m_functions.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "no mathing function was found");
            
            return *it->second;
        }
        void init_functions()
        {
            if(!m_functions.empty())
            {
                for (auto &functionPair : m_functions)
                {
                    auto descriptor = functionPair.second->to_python();
                    std::size_t hashCode = functionPair.second->get_hash_code();
    
                    ObjectPtr self(PyTuple_New(2), &Deleter::Owner); //self = name and object, function will release the tuple
                    
                    ObjectPtr context(PyCapsule_New(m_context.get(), nullptr, nullptr), &Deleter::Owner);
                    CPYTHON_VERIFY(context.get() != nullptr, "Encapsulating module context failed");
                    PyTuple_SetItem(self.get(), 0, context.release());
                    
                    ObjectPtr hashCodePtr(PyLong_FromUnsignedLong(hashCode), &Deleter::Owner);
                    PyTuple_SetItem(self.get(), 1, hashCodePtr.release());
                    
                    std::string name = descriptor->ml_name;
                    ObjectPtr cFunction(PyCFunction_NewEx(descriptor.release(), self.get(), NULL), &Deleter::Owner);
                    CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), name.c_str(), cFunction.release()) == 0, "global function registration with module failed");
                    
                    m_context->add_function(hashCode, std::move(functionPair.second));
                }
            }
        }
        void add_enum(const std::string& name, ObjectPtr&& dictionary)
        {
            m_enums.emplace_back(name, std::move(dictionary));
        }
        PyObject* get_module() const{ return m_module.get(); }
        void finalize()
        {
            char* name = new char[m_name.length() + 1];
            std::copy_n(m_name.c_str(), m_name.length(), name);
            name[m_name.length()] = '\0';
    
            char* doc = new char[m_doc.length() + 1];
            std::copy_n(m_doc.c_str(), m_doc.length(), doc);
            doc[m_doc.length()] = '\0';
            
            m_moduleDef->m_base = PyModuleDef_HEAD_INIT;
            m_moduleDef->m_name = name;
            m_moduleDef->m_doc = doc;
            m_moduleDef->m_free = &free_module;
            m_module.reset(PyModule_Create(m_moduleDef.get()));
            CPYTHON_VERIFY(m_module.get() != nullptr, "Module registration failed");
            m_moduleDef.release();
    
            init_functions();
            init_types();
            init_variables();
            init_enums();
    
            ObjectPtr context(PyCapsule_New(m_context.get(), nullptr, nullptr), &Deleter::Owner);
            CPYTHON_VERIFY(context.get() != nullptr, "Encapsulating module context failed");
            m_context.release();
            CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), "context", context.release()) == 0, "module context registration with module failed");
        }

    private:
        void init_variables()
        {
            if(m_variables.empty() == false)
            {
                for(auto& variable : m_variables) {
                    auto var = variable->to_python();
                    CPYTHON_VERIFY(PyModule_AddObject((PyObject *) m_module.get(), variable->get_name().c_str(),
                                                      var.release()) == 0,
                                   "Type registration with module failed");
                }
            }
        }
        void init_types()
        {
            if(!m_types.empty())
            {
                for(auto& typePair : m_types)
                {
                    ObjectPtr& type = typePair.second;
                    CPythonType& clazzType = *CPythonType::get_type(type.get());
                    CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), clazzType.get_name().c_str(), type.release()) == 0, "Type registration with module failed"); //Module added ref count ownership was taken by the module python's version
                }
            }
        }
        void init_enums()
        {
            if(m_enums.empty() == false)
            {
                for(auto& enumPair : m_enums)
                {
                    ObjectPtr name(PyUnicode_FromString(enumPair.first.c_str()), &Deleter::Owner);
                    CPYTHON_VERIFY(name != nullptr, "Was unable to import enum name into python");
                    ObjectPtr enumClass = Python::invoke_function("enum", "Enum", "EnumMeta._create_", name.get(), enumPair.second.get());
                    CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), enumPair.first.c_str(), enumClass.release()) == 0, "Type registration with module failed");
                }
            }
        }
        static void free_module(void *ptr)
        {
            auto objectPtr = (PyObject*)ptr;
            Dictionary moduleDict(PyModule_GetDict(objectPtr));
            PyModuleDef& moduleDef = *PyModule_GetDef(objectPtr);
            delete [] moduleDef.m_name;
            delete [] moduleDef.m_doc;
            
            std::vector<PyTypeObject*> types;
            for(auto& elem : moduleDict)
            {
                auto object = elem.get<PyObject*>();
                if(object->ob_type->ob_base.ob_size == MetaClass::get_check_sum())
                    types.emplace_back(reinterpret_cast<PyTypeObject*>(object));
            }
            
            auto contextCapsule = moduleDict.get<ObjectPtr, const char*>("context");
            auto context = reinterpret_cast<ModuleContext*>(PyCapsule_GetPointer(contextCapsule.get(), nullptr));
            delete context;
            
            moduleDict.clear();
            
            for(auto& type : types)
            {
                Dictionary dict(type->tp_dict);
                dict.clear();
    
                Py_XDECREF(type->tp_mro);
                type->tp_mro = nullptr;
                free((void*)type->tp_doc);
                type->tp_doc = nullptr;
    
                type->ob_base.ob_base.ob_refcnt -= 2;
                PyTypeObject* meta = type->ob_base.ob_base.ob_type;
                meta->tp_dealloc(reinterpret_cast<PyObject*>(type));
            }
        }

    private:
        std::unique_ptr<PyModuleDef> m_moduleDef;
        ObjectPtr m_module;
        std::unique_ptr<ModuleContext> m_context;
        typedef std::unordered_map<TypeKey, ObjectPtr> Types;
        Types m_types;
        std::vector<Variable::VariablePtr> m_variables;
        typedef std::unordered_map<std::size_t, FunctionPtr> Functions;
        Functions m_functions;
        typedef std::pair<std::string, ObjectPtr> EnumPair;
        std::vector<EnumPair> m_enums;
        std::string m_name;
        std::string m_doc;
    };
}

