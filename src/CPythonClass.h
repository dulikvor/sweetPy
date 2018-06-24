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
#include "CPythonType.h"
#include "Exception.h"

namespace pycppconn {

    class IMemberAccessor
    {
    public:
        virtual ~IMemberAccessor(){}
        virtual void Set(PyObject* object, PyObject* rhs) = 0;
    };

    template<typename MemberType>
    class MemberAccessor : public IMemberAccessor
    {
    public:
        MemberAccessor(int offset): m_offset(offset){}
        virtual ~MemberAccessor(){}
        /*
         * Set will receive the rhs object, the object may be a reference wrapper
         * /a python supported type/a C++ type.
         */
        void Set(PyObject* object, PyObject* rhs) override
        {
            MemberType& member = *(MemberType*)((char*)object + m_offset);
            if(CPythonRef<>::IsReferenceType<MemberType>(rhs))
            {
                MemberType& _rhs = Object<MemberType&>::FromPython(rhs);
                member = _rhs;
            }
            else
            {
                auto _rhs = Object<MemberType>::FromPython(rhs);
                member = _rhs;
            }
        }

    private:
        int m_offset;
    };

    template<typename Type>
    class CPythonClassType : public CPythonType
    {
    public:
        typedef CPythonClassType<Type> self;

        CPythonClassType(const std::string& name, const std::string& doc)
            :CPythonType(name, doc)
        {
            ob_type = &CPythonMetaClass<>::GetStaticMetaType();
            ob_refcnt = 1;
            ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(Type) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_CLASS |
                       Py_TPFLAGS_HAVE_GC;
            tp_doc = m_doc.c_str();
            tp_traverse = &Traverse;
            tp_new = PyBaseObject_Type.tp_new;
            tp_setattro = &SetAttribute;

        }

        template<typename MemberType>
        void CreateMemberAccessor(int offset)
        {
            if(m_membersAccessors.find(offset) != m_membersAccessors.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "MemberAccessor for that given offset already exists  - %d", offset);
            m_membersAccessors.insert({offset, std::shared_ptr<IMemberAccessor>(new MemberAccessor<MemberType>(offset))});
        }

        IMemberAccessor& GetAccessor(int offset)
        {
            auto it = m_membersAccessors.find(offset);
            if(it == m_membersAccessors.end())
                throw CPythonException(PyExc_LookupError, __CORE_SOURCE, "Requested accessor couldn't be found, offset - %d", offset);
            return *it->second;
        }



    private:
        static int Traverse(PyObject *object, visitproc visit, void *arg)
        {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
            return 0;
        }

        static void Dealloc(PyObject *object)
        {
            //No need to call reference forget - being called by _Py_Dealloc
            PyTypeObject* type = Py_TYPE(object);

            if (PyType_IS_GC(type))
                PyObject_GC_UnTrack(object);

            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_DECREF(type);

            ((Type*)(object + 1))->~Type();
            type->tp_free(object);
        }

        static int SetAttribute(PyObject* object, PyObject* attrName, PyObject* value) {
            PyTypeObject* type = CPyModuleContainer::Instance().GetType(CPyModuleContainer::TypeHash<self>());
            CPYTHON_VERIFY(type != nullptr,"was unable to locate type");
            CPYTHON_VERIFY(attrName->ob_type == &PyString_Type,"attrName must be py string type");
            char* name = PyString_AsString(attrName);
            MembersDefs defs(type->tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs){return strcmp(rhs.name, name) == 0; });
            if(it == defs.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Requested attribute - %s, was not found", name);

            self& cpythonClassType = static_cast<self&>(*type);
            IMemberAccessor& accessor = cpythonClassType.GetAccessor(it->offset);
            accessor.Set(object, value);
            return 0;
        }


    private:
        std::unordered_map<int, std::shared_ptr<IMemberAccessor>> m_membersAccessors; //Nothing is shared, but due to the fact vptr is not allowed we will use raii to keep the memory in check.
    };


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
            type.template CreateMemberAccessor<MemberType>(GetOffset(member));
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
