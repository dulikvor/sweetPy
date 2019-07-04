#pragma once

#include <Python.h>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>
#include <structmember.h>
#include "Core/Traits.h"
#include "Core/Lock.h"
#include "Core/Exception.h"
#include "Core/Assert.h"
#include "Types/ObjectPtr.h"
#include "src/Detail/MetaClass.h"
#include "src/Detail/ClazzPyType.h"
#include "src/Detail/TypesContainer.h"
#include "src/Detail/Constructor.h"
#include "src/Detail/ReferenceType.h"
#include "src/Detail/Member.h"
#include "src/Detail/Function.h"
#include "src/Detail/TypedMemberAccessor.h"
#include "Module.h"

namespace sweetPy {
    
    template<typename T, typename _T = decay_t<T>>
    class PlainClazz
    {
    private:
        typedef ClazzPyType<_T> PyType;
        
    public:
        PlainClazz(Module &module, const std::string &name, const std::string &doc)
                : m_module(module),
                  m_type(CPythonType::get_py_object(new PyType(name, doc, &free_type)), &Deleter::Owner),
                  m_context(static_cast<PyType*>(CPythonType::get_type(m_type.get()))->get_context())
        {
        }
        
        ~PlainClazz()
        {
            auto& type = *CPythonType::get_type(m_type.get());
            if(!m_module.is_type_exists(type.get_hash_code()))
            {
                PyType_Ready(reinterpret_cast<PyTypeObject*>(m_type.get()));
                clear_trace_ref();
                TypesContainer::instance().add_type(type.get_hash_code(), type, false);
                m_module.add_type(type.get_hash_code(), std::move(m_type), false);
            }
        }
        
    private:
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REFS
            auto type = reinterpret_cast<PyTypeObject*>(m_type.get());
            if(type->ob_base.ob_base._ob_prev)
                type->ob_base.ob_base._ob_prev->_ob_next = type->ob_base.ob_base._ob_next;
            if(type->ob_base.ob_base._ob_next)
                type->ob_base.ob_base._ob_next->_ob_prev = type->ob_base.ob_base._ob_prev;
        
            type->ob_base.ob_base._ob_next = NULL;
            type->ob_base.ob_base._ob_prev = NULL;
#endif
        }
        static void free_type(void* ptr)
        {
            delete reinterpret_cast<PyType*>(ptr);
        }
        
    private:
        Module &m_module;
        ObjectPtr m_type;
        ClazzContext& m_context;
    };
    

    template<typename T, typename _T = decay_t<T>>
    class Clazz
    {
    private:
        typedef ClazzPyType<_T> PyType;
        template<typename...>
        struct TypesInitializer{};
        template<typename... Args>
        struct TypesInitializer<std::tuple<Args...>>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                invoker(PlainClazz<Args>(module, namePrefix + std::to_string(typeid(Args).hash_code()), "")...);
            }
        };
    
        template<typename...>
        struct ReferenceTypesInitializer{};
        template<typename... Args>
        struct ReferenceTypesInitializer<std::tuple<Args...>>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                invoker(PlainReferenceType<Args>(module, namePrefix + std::to_string(typeid(Args).hash_code()), "")...);
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
        Clazz(Module &module, const std::string &name, const std::string &doc, bool forceInsertion = true)
                : m_module(module),
                  m_type(CPythonType::get_py_object(new PyType(name, doc, &free_type)), &Deleter::Owner),
                  m_context(static_cast<PyType*>(CPythonType::get_type(m_type.get()))->get_context()),
                  m_forceInsertion(forceInsertion)
        {
        }

        ~Clazz()
        {
            init_members();
            init_methods();
            init_static_methods();
    
            CPythonType& type = *CPythonType::get_type(m_type.get());
            if(type.ht_type.tp_init == nullptr)
                add_constructor<>();
    
            PyType_Ready(&type.ht_type);
            clear_trace_ref();
            ReferenceType<_T> refType(m_module, std::string(type.get_name()) + "_ref", "", m_context);
            
            TypesContainer::instance().add_type(type.get_hash_code(), type, m_forceInsertion);
            m_module.add_type(type.get_hash_code(), std::move(m_type));
        }

        template<typename X, typename = enable_if_t<std::is_member_function_pointer<X>::value>>
        void add_method(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef MemberFunction<_T, X> FuncType;
            m_memberFunctions.emplace_back(new FuncType(name, doc, memberFunction));
            FunctionTypesInitializer<X>::initialize_types(m_module, "Clazz");
        }
    
        template<typename X, typename = enable_if_t<is_function_pointer<X>::value>>
        void add_static_method(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef StaticFunction<_T, X> FuncType;
            m_memberStaticFunctions.emplace_back(new FuncType(name, doc, memberFunction));
            FunctionTypesInitializer<X>::initialize_types(m_module, "Clazz");
        }
    
        template<typename... Args>
        enable_if_t<!std::is_constructible<_T, Args...>::value>
        add_constructor()
        {
            throw CPythonException(PyExc_PermissionError, __CORE_SOURCE, "No sutiable constructor");
        }

        template<typename... Args>
        enable_if_t<std::is_constructible<_T, Args...>::value>
        add_constructor()
        {
            reinterpret_cast<PyTypeObject*>(m_type.get())->tp_init = &Constructor<_T, Args...>::wrapper;
        }

        template<typename X>
        void add_member(const std::string &name, X _T::* member, const std::string &doc)
        {
            m_members.emplace_back(new TypedMember<_T, X>(name, member, doc));
            int offset = get_offset(member);
            ClazzContext::MemberAccessorPtr accessor(new TypedMemberAccessor<X>(offset));
            m_context.add_member(offset, std::move(accessor));
        }
        
    private:
        void init_methods()
        {
            if(m_memberFunctions.empty() == false)
            {
                PyMethodDef *methods = new PyMethodDef[m_memberFunctions.size() + 1]; //spare space for sentinal
                reinterpret_cast<PyTypeObject*>(m_type.get())->tp_methods = methods;
                for (auto &method : m_memberFunctions)
                {
                    *methods = *method->to_python();
                    std::size_t hash_code = method->get_hash_code();
                    m_context.add_member_function(hash_code, ClazzContext::FunctionPtr(method.release()));
                    methods++;
                }
                *methods = {NULL, NULL, 0, NULL};
            }
        }
        void init_static_methods()
        {
            if (m_memberStaticFunctions.empty() == false)
            {
                auto type = ObjectPtr(CPythonType::get_py_object(new MetaClass(
                        std::string(CPythonType::get_type(m_type.get())->get_name()) + "MetaClass", "")), &Deleter::Owner);
                CPythonType::get_type(m_type.get())->ht_type.ob_base.ob_base.ob_type = reinterpret_cast<PyTypeObject*>(type.get());
                auto& metaClass = static_cast<MetaClass&>(*reinterpret_cast<PyHeapTypeObject*>(type.get()));
                for (auto &staticFunction : m_memberStaticFunctions)
                {
                    metaClass.add_static_method(std::move(staticFunction));
                }
                metaClass.finalize();
                m_module.add_type(metaClass.get_hash_code(), std::move(type));
            }
        }
        void init_members()
        {
            if( m_members.empty() == false)
            {
                PyMemberDef *members = new PyMemberDef[m_members.size() + 1]; //spare space for sentinal
                CPythonType::get_type(m_type.get())->ht_type.tp_members = members;
                for (const auto &member : m_members)
                {
                    *members = *member->to_python();
                    members++;
                }
                *members = {NULL, 0, 0, 0, NULL};
            }
        }
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REFS
            
            auto type = reinterpret_cast<PyTypeObject*>(m_type.get());
            if(type->ob_base.ob_base._ob_prev)
                type->ob_base.ob_base._ob_prev->_ob_next = type->ob_base.ob_base._ob_next;
            if(type->ob_base.ob_base._ob_next)
                type->ob_base.ob_base._ob_next->_ob_prev = type->ob_base.ob_base._ob_prev;
    
            type->ob_base.ob_base._ob_next = NULL;
            type->ob_base.ob_base._ob_prev = NULL;
#endif
        }
        static void free_type(void* ptr)
        {
            delete reinterpret_cast<PyType*>(ptr);
        }

    private:
        typedef std::unique_ptr<Function> FunctionPtr;
        typedef std::vector<FunctionPtr> MemberFunctions;
        MemberFunctions m_memberFunctions;
        MemberFunctions m_memberStaticFunctions;
        
        typedef std::unique_ptr<Member> MemberPtr;
        typedef std::vector<MemberPtr> Members;
        Members m_members;
        Module &m_module;
        ObjectPtr m_type;
        ClazzContext& m_context;
        bool m_forceInsertion;
    };
}
