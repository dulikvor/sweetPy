#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <Python.h>
#include "core/Source.h"
#include "CPyModuleContainer.h"
#include "CPythonMember.h"
#include "ICPythonFunction.h"
#include "CPythonModule.h"
#include "CPythonType.h"
#include "CPythonMetaClass.h"

namespace sweetPy {

    template<typename Type, typename std::enable_if<!std::is_reference<Type>::value, bool>::type = true>
    class CPythonRefObject {
    public:
        CPythonRefObject(Type& object) : m_object(object){}
        Type& GetRef(){ return m_object; }
    private:
        Type& m_object;
    };


    template<typename Type>
    class CPythonRefType : public CPythonType
    {
    public:
        CPythonRefType(const std::string& name, const std::string& doc)
                : CPythonType(name, doc)
        {
            ob_type = &CPythonMetaClass<>::GetStaticMetaType();
            ob_refcnt = 1;
            ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(CPythonRefObject<Type>) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_CLASS |
                       Py_TPFLAGS_HAVE_GC;
            tp_doc = m_doc.c_str();
            tp_traverse = &Traverse;
            tp_new = PyBaseObject_Type.tp_new;
        }

    private:
        static void Dealloc(PyObject *object)
        {
            //No need to call reference forget - being called by _Py_Dealloc
            PyTypeObject* type = Py_TYPE(object);

            if (PyType_IS_GC(type))
                PyObject_GC_UnTrack(object);

            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_DECREF(type);

            type->tp_free(object);
        }

        static int Traverse(PyObject *self, visitproc visit, void *arg)
        {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
            return 0;
        }
    };


    template<typename Type = void*, typename std::enable_if<!std::is_reference<Type>::value, bool>::type = true>
    class CPythonRef
    {
    public:
        CPythonRef(CPythonModule &module, const std::string &name, const std::string &doc)
        : m_module(module), m_type( new CPythonRefType<Type>(name, doc))
        {
            Py_IncRef((PyObject *) m_type.get()); //Making sure the true owner of the type is CPythonClass
        }

        ~CPythonRef()
        {
            InitMembers();
            InitMethods();
            PyType_Ready(m_type.get());
            CPyModuleContainer::Instance().AddType(CPyModuleContainer::TypeHash<CPythonRefType<Type>>(), m_type.get());
            m_module.AddType(std::move(m_type));
        }

        static void InitStaticType()
        {
            //Only one instance of CPythonRef static type may exists
            if(std::is_same<Type, void*>::value == false)
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "InitStaticType is not support by current type");
            PyType_Ready(&m_staticType);
        }

        static PyTypeObject& GetStaticType()
        {
            //Only one instance of CPythonRef static type may exists
            if(std::is_same<Type, void*>::value == false)
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "GetStaticType is not support by current type");
            return m_staticType;
        }

        template<typename T, typename std::enable_if<!std::is_reference<T>::value, bool>::type = true>
        static bool IsReferenceType(PyObject* obj)
        {
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<T>>();
            return obj->ob_type == &CPythonRef<>::GetStaticType() || obj->ob_type == CPyModuleContainer::Instance().GetType(key);
        }

        template<typename T>
        static PyObject* Alloc(PyTypeObject* type, T&& reference)
        {
            PyObject* object = type->tp_alloc(type, 0);
            new(object + 1)CPythonRefObject<typename std::remove_reference<T>::type>(std::forward<T>(reference));
            return object;
        }

        void AddMethods(const std::vector<std::shared_ptr<ICPythonFunction>>& methods){ m_cPythonMemberFunctions = methods; }
        void AddMembers(const std::vector<std::shared_ptr<ICPythonMember>>& members){ m_cPythonMembers = members; }

        void InitMethods()
        {
            if(m_cPythonMemberFunctions.empty() == false)
            {
                PyMethodDef *methods = new PyMethodDef[m_cPythonMemberFunctions.size() + 1]; //spare space for sentinal
                m_type->tp_methods = methods;
                for (const auto &method : m_cPythonMemberFunctions) {
                    *methods = *method->ToPython();
                    methods++;
                }
                *methods = {NULL, NULL, 0, NULL};
            }
        }

        void InitMembers()
        {
            if(m_cPythonMembers.empty() == false)
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
        static PyTypeObject m_staticType;
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::vector<std::shared_ptr<ICPythonMember>> m_cPythonMembers;
        std::unique_ptr<CPythonType> m_type;
        CPythonModule &m_module;
    };
}