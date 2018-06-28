#pragma once

#include <typeinfo>
#include <type_traits>
#include <vector>
#include <memory>
#include <unordered_map>
#include <Python.h>
#include <structmember.h>
#include "Lock.h"
#include "CPythonFunction.h"
#include "CPythonMember.h"
#include "CPyModuleContainer.h"
#include "CPythonModule.h"
#include "CPythonMetaClass.h"
#include "CPythonConstructor.h"
#include "CPythonRef.h"
#include "CPythonObject.h"
#include "CPythonClassType.h"
#include "IMemberAccessor.h"
#include "MemberAccessor.h"
#include "Exception.h"

namespace pycppconn {

    template<typename T, typename Type = typename std::remove_const<typename std::remove_pointer<
            typename std::remove_reference<T>::type>::type>::type>
    class CPythonClass {
    public:
        CPythonClass(CPythonModule &module, const std::string &name, const std::string &doc)
                : m_module(module), m_type(new CPythonClassType<Type>(name, doc)) {
            Py_IncRef((PyObject *)m_type.get()); //Making sure the true owner of the type is CPythonClass
        }

        ~CPythonClass() {
            InitMembers();
            InitMethods();
            InitStaticMethods();
            PyType_Ready(m_type.get());
            CPyModuleContainer::Instance().AddType(CPyModuleContainer::TypeHash<CPythonClassType<Type>>(), m_type.get());
            //Init ref type
            CPythonRef<Type> refType(m_module, std::string(m_type->GetName()) + "_ref", std::string(m_type->GetDoc()) + "_ref");
            refType.AddMethods(m_cPythonMemberFunctions);
            refType.AddMembers(m_cPythonMembers);

            m_module.AddType(std::move(m_type));
        }

        template<typename X, typename std::enable_if<std::is_member_function_pointer<X>::value, bool>::type = true>
        void AddMethod(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef CPythonFunction<Type, X> CPyFuncType;
            m_cPythonMemberFunctions.emplace_back(new CPyFuncType(name, doc, memberFunction));
            CPyModuleContainer::Instance().AddMethod(GenerateMethodId<CPyFuncType>(), m_cPythonMemberFunctions.back());
        }

        template<typename X, typename std::enable_if<std::is_function<typename std::remove_pointer<X>::type>::value, bool>::type = true>
        void AddStaticMethod(const std::string &name, const std::string &doc, X &&memberFunction) {
            typedef CPythonFunction<Type, X> CPyFuncType;
            m_cPythonMemberStaticFunctions.emplace_back(new CPyFuncType(name, doc, memberFunction));
            CPyModuleContainer::Instance().AddStaticMethod(GenerateMethodId<CPyFuncType>(),
                                                           m_cPythonMemberStaticFunctions.back());
        }

        template<typename... Args>
        void AddConstructor() {
            m_type->tp_init = &CPythonConstructor<Type, Args...>::Wrapper;
        }

        template<typename MemberType>
        void AddMember(const std::string &name, MemberType Type::* member, const std::string &doc)
        {
            m_cPythonMembers.emplace_back(new CPythonMember<Type, MemberType>(name, member, doc));
            CPythonClassType<Type>& type = static_cast<CPythonClassType<Type>&>(*m_type);
            int offset = GetOffset(member);
            std::unique_ptr<IMemberAccessor> accessor(new MemberAccessor<MemberType>(offset));
            type.CreateMemberAccessor(offset, std::move(accessor));
        }

    private:
        template<typename CPythonFunctionType>
        static int GenerateMethodId() {
            return typeid(CPythonFunctionType).hash_code();
        }


        void InitMethods() {
            if(m_cPythonMemberFunctions.empty() == false)
            {
                PyMethodDef *methods = new PyMethodDef[m_cPythonMemberFunctions.size() + 1]; //spare space for sentinal
                m_type->tp_methods = methods;
                for (const auto &method : m_cPythonMemberFunctions) {
                    method->AllocateObjectsTypes(m_module);
                    *methods = *method->ToPython();
                    methods++;
                }
                *methods = {NULL, NULL, 0, NULL};
            }
        }


        void InitStaticMethods() {
            if (m_cPythonMemberStaticFunctions.size() > 0) {
                auto type = std::unique_ptr<CPythonMetaClass<>>(new CPythonMetaClass<>(m_module,
                                                                                   std::string(m_type->GetName()) +
                                                                                   "MetaClass",
                                                                                   std::string(m_type->GetDoc()) +
                                                                                   "MetaClass"));
                m_type.get()->ob_type = &type->ToPython();
                for (auto &staticFunction : m_cPythonMemberStaticFunctions) {
                    staticFunction->AllocateObjectsTypes(m_module);
                    type->AddMethod(staticFunction);
                }
                type->InitType();
            }
        }

        void InitMembers() {
            if( m_cPythonMembers.empty() == false)
            {
                PyMemberDef *members = new PyMemberDef[m_cPythonMembers.size() + 1]; //spare space for sentinal
                m_type->tp_members = members;
                for (const auto &member : m_cPythonMembers) {
                    *members = *member->ToPython();
                    members++;
                }
                *members = {NULL, 0, 0, 0, NULL};
            }
        }

    private:
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberStaticFunctions;
        std::vector<std::shared_ptr<ICPythonMember>> m_cPythonMembers;
        std::unique_ptr<CPythonType> m_type;
        CPythonModule &m_module;
    };
}
