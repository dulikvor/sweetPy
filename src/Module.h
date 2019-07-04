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
#include "Types/ObjectPtr.h"
#include "Types/Dictionary.h"
#include "src/Detail/CPythonType.h"
#include "src/Detail/CPythonObject.h"
#include "src/Detail/Function.h"
#include "src/Detail/ConcreteFunction.h"
#include "src/Detail/ModuleContext.h"

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
                    
                    ObjectPtr cFunction(PyCFunction_NewEx(descriptor.get(), self.get(), NULL), &Deleter::Owner);
                    CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), descriptor->ml_name, cFunction.release()) == 0, "global function registration with module failed");
                    
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
            CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), "context", context.release()) == 0, "module context registration with module failed");
            m_context.release();
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

