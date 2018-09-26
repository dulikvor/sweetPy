#pragma once

#include <Python.h>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include "core/Source.h"
#include "Core/Exception.h"
#include "Core/Dictionary.h"
#include "CPyModuleContainer.h"
#include "CPythonMember.h"
#include "CPythonFunction.h"
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
        typedef CPythonRefType<Type> self;

        CPythonRefType(const std::string& name, const std::string& doc)
                : CPythonType(name, doc)
        {
            ob_base.ob_base.ob_type = &CPythonMetaClass::GetStaticMetaType();
            ob_base.ob_base.ob_refcnt = 1;
            ob_base.ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(CPythonRefObject<Type>) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_GC;
            tp_doc = m_doc.c_str();
            tp_traverse = &Traverse;
            tp_new = PyBaseObject_Type.tp_new;
            tp_getattro = &GetAttribute;
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

        static PyObject* GetAttribute(PyObject *object, PyObject *attrName)
        {
            Dictionary dictionary(object);
            object_ptr descriptor = dictionary.GetObject(attrName);
            if(descriptor.get() != nullptr && descriptor->ob_type == &PyMethodDescr_Type)
                return GetMethod(descriptor, object);
            else
                return  PyObject_GenericGetAttr(object, attrName);
        }

        static PyObject* GetMethod(const object_ptr& descr, PyObject *obj)
        {
            PyMethodDescrObject& descriptor = static_cast<PyMethodDescrObject&>(*(PyMethodDescrObject*)descr.get());
            object_ptr self(PyTuple_New(2), &Deleter::Owner); //self = name and object, function will release the tuple
            Py_XINCREF(obj); //tuple steals the reference
            PyTuple_SetItem(self.get(), 0, obj);
            Py_XINCREF(descriptor.d_common.d_name); //tuple steals the reference
            PyTuple_SetItem(self.get(), 1, descriptor.d_common.d_name);
            return PyCFunction_NewEx(descriptor.d_method, self.get(), NULL);
        }
    };


    template<typename Type = void*, typename std::enable_if<!std::is_reference<Type>::value, bool>::type = true>
    class CPythonRef
    {
    private:
        struct NonCallableRefType : public PyTypeObject
        {
        public:
            NonCallableRefType();
        };

    public:
        CPythonRef(CPythonModule &module, const std::string &name, const std::string &doc)
        : m_module(module), m_type((PyObject*)new CPythonRefType<Type>(name, doc), &Deleter::Owner)
        {
            Py_IncRef((PyObject *) m_type.get()); //Making sure the true owner of the type is CPythonClass
        }

        ~CPythonRef()
        {
            InitMembers();
            InitMethods();
            PyType_Ready((PyTypeObject*)m_type.get());
            m_module.AddType((CPythonType*)m_type.get());
            CPyModuleContainer::Instance().AddType(CPyModuleContainer::TypeHash<CPythonRefType<Type>>(), std::move(m_type));
        }

        static void InitStaticType()
        {
            //Only one instance of CPythonRef static type may exists
            if(std::is_same<Type, void*>::value == false)
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "InitStaticType is not support by current type");
            PyType_Ready(&m_staticType);
        }

        static NonCallableRefType& GetStaticType()
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
            return obj->ob_type == CPyModuleContainer::Instance().GetType(key);
        }

        template<typename T>
        static PyObject* Alloc(PyTypeObject* type, T&& reference)
        {
            PyObject* object = type->tp_alloc(type, 0);
            new(object + 1)CPythonRefObject<typename std::remove_reference<T>::type>(std::forward<T>(reference));
            return object;
        }

        void AddMethods(const std::vector<std::shared_ptr<CPythonFunction>>& methods){ m_memberFunctions = methods; }
        void AddMembers(const std::vector<std::shared_ptr<ICPythonMember>>& members){ m_members = members; }

        void InitMethods()
        {
            if(m_memberFunctions.empty() == false)
            {
                PyMethodDef *methods = new PyMethodDef[m_memberFunctions.size() + 1]; //spare space for sentinal
                ((PyTypeObject*)m_type.get())->tp_methods = methods;
                for (const auto &method : m_memberFunctions) {
                    *methods = *method->ToPython();
                    methods++;
                }
                *methods = {NULL, NULL, 0, NULL};
            }
        }

        void InitMembers()
        {
            if(m_members.empty() == false)
            {
                PyMemberDef *members = new PyMemberDef[m_members.size() + 1]; //spare space for sentinal
                ((PyTypeObject*)m_type.get())->tp_members = members;
                for (const auto &member : m_members) {
                    *members = *member->ToPython();
                    members++;
                }
                *members = {NULL, 0, 0, 0, NULL};
            }
        }

    private:
        static NonCallableRefType m_staticType;
        std::vector<std::shared_ptr<CPythonFunction>> m_memberFunctions;
        std::vector<std::shared_ptr<ICPythonMember>> m_members;
        object_ptr m_type;
        CPythonModule &m_module;
    };
}