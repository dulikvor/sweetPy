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
#include "Core/SPException.h"
#include "Core/Assert.h"
#include "Types/ObjectPtr.h"
#include "Detail/MetaClass.h"
#include "Detail/ClazzPyType.h"
#include "Detail/TypesContainer.h"
#include "Detail/Constructor.h"
#include "Detail/ReferenceType.h"
#include "Detail/Member.h"
#include "Detail/Function.h"
#include "Detail/ConcreteFunction.h"
#include "Detail/TypedMemberAccessor.h"
#include "Detail/Object.h"
#include "Module.h"

namespace sweetPy {

    template<typename T>
    class Clazz
    {
    private:
        typedef Clazz<T> Self;
        typedef ClazzPyType<T> PyType;
        template<typename...>
        struct TypesInitializer{};
        template<typename... Args>
        struct TypesInitializer<std::tuple<Args...>>
        {
            static void initialize_types(Module& module, const std::string& namePrefix)
            {
                invoker(initialize_type<Args>(module, namePrefix)...);
            }
            template<typename X>
            static void* initialize_type(Module& module, const std::string& namePrefix)
            {
                auto objectDestroyer = [](PyObject* object){
                    X* instance = &ClazzObject<X>::get_val(object);
                    instance->~X();
                };
                std::size_t hashCode = Hash::generate_hash_code<X>();
            
                std::unique_ptr<PyObject, std::function<void(PyObject*)>> plainType(
                        CPythonType::get_py_object(
                                new PlainType(namePrefix + std::to_string(hashCode), "", ClazzObject<X>::get_size(),
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
    
                                //mro is pre released in order to prevent the invocation of - python PyObject release routine,
                                // which will inspect the type ref list under debug variation (our types are not part of the ref list).
                                Py_XDECREF(type->tp_mro);
                                type->tp_mro = nullptr;
                                free((void*)type->tp_doc);
                                //documents are extended beyond the original data in debug mode which our document lacks, due to that
                                //free will be initiated explicitly prior for deallocating the type.
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
            template<typename X, typename _X = remove_reference_t<X>>
            static void* initialize_type(Module& module, const std::string& namePrefix)
            {
                std::size_t hashCode = Hash::generate_hash_code<ReferenceObject<_X>>();
                std::unique_ptr<PyObject, std::function<void(PyObject*)>> plainType(
                        CPythonType::get_py_object(
                                new PlainType(namePrefix + std::to_string(hashCode), "", ClazzObject<ReferenceObject<_X>>::get_size(),
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
                                //mro is pre released in order to prevent the invocation of - python PyObject release routine,
                                // which will inspect the type ref list under debug variation (our types are not part of the ref list).
                                Py_XDECREF(type->tp_mro);
                                type->tp_mro = nullptr;
                                //documents are extended beyond the original data in debug mode which our document lacks, due to that
                                //free will be initiated explicitly prior for deallocating the type.
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
                ReferenceTypesInitializer<byReference>::initialize_types(module, namePrefix + "R");
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
                ReferenceTypesInitializer<byReference>::initialize_types(module, namePrefix + "R");
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
            if(type.ht_type.tp_init == nullptr && std::is_constructible<T>::value)
                add_constructor<>();
            
            PyType_Ready(&type.ht_type);
            type.clear_trace_ref();
            ReferenceType<T> refType(m_module, std::string(type.get_name()) + "_ref", "", m_context);
            ReferenceType<T const> refConstType(m_module, std::string(type.get_name()) + "_const_ref", "", m_context);
            
            TypesContainer::instance().add_type(type.get_hash_code(), type, m_forceInsertion);
            m_module.add_type(type.get_hash_code(), std::move(m_type));
        }

        template<typename X, typename = enable_if_t<std::is_member_function_pointer<X>::value>>
        void add_method(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef MemberFunction<T, X> FuncType;
            m_memberFunctions.emplace_back(new FuncType(name, doc, memberFunction));
            FunctionTypesInitializer<X>::initialize_types(m_module, "C");
        }
    
        template<typename X, typename = enable_if_t<is_function_pointer<X>::value>>
        void add_static_method(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef StaticFunction<T, X> FuncType;
            m_memberStaticFunctions.emplace_back(new FuncType(name, doc, memberFunction));
            FunctionTypesInitializer<X>::initialize_types(m_module, "C");
        }
    
        template<typename... Args>
        enable_if_t<!std::is_constructible<T, Args...>::value>
        add_constructor()
        {
            throw CPythonException(PyExc_PermissionError, __CORE_SOURCE, "No sutiable constructor");
        }

        template<typename... Args>
        enable_if_t<std::is_constructible<T, Args...>::value>
        add_constructor()
        {
            reinterpret_cast<PyTypeObject*>(m_type.get())->tp_init = &Constructor<T, Args...>::wrapper;
        }

        template<typename X>
        void add_member(const std::string &name, X T::* member, const std::string &doc)
        {
            m_members.emplace_back(new TypedMember<T, X>(name, member, doc));
            int offset = get_offset(member);
            ClazzContext::MemberAccessorPtr accessor(new TypedMemberAccessor<Self, X>(offset));
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
        static void free_type(PyObject* ptr)
        {
            delete static_cast<PyType*>(CPythonType::get_type(ptr));
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
