#pragma once

#include <string>
#include <Python.h>
#include "CPythonType.h"
#include "IMemberAccessor.h"
#include "Core/Exception.h"
#include "Core/Assert.h"
#include "Core/Dictionary.h"


namespace sweetPy {

    template<typename Type>
    class CPythonClassType : public CPythonType {
    public:
        typedef CPythonType::CPythonTypeHash<Type> hash_type;
        typedef CPythonClassType<Type> self;

        CPythonClassType(const std::string &name, const std::string &doc)
                : CPythonType(name, doc) {
            ob_base.ob_base.ob_type = &CPythonMetaClass::GetStaticMetaType();
            ob_base.ob_base.ob_refcnt = 1;
            ob_base.ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(Type) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_GC;
            tp_doc = m_doc.c_str();
            tp_traverse = &Traverse;
            tp_new = PyBaseObject_Type.tp_new;
            tp_setattro = &SetAttribute;
            tp_getattro = &GetAttribute;
        }

        //Due to dependency with CPythonObject, the member will be created by CPythonClass
        void CreateMemberAccessor(int offset, std::unique_ptr<IMemberAccessor>&& accessor) {
            if (m_membersAccessors.find(offset) != m_membersAccessors.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE,
                                       "MemberAccessor for that given offset already exists  - %d", offset);
            m_membersAccessors.insert(
                    {offset, std::shared_ptr<IMemberAccessor>(accessor.release())});
        }

        IMemberAccessor &GetAccessor(int offset) {
            auto it = m_membersAccessors.find(offset);
            if (it == m_membersAccessors.end())
                throw CPythonException(PyExc_LookupError, __CORE_SOURCE,
                                       "Requested accessor couldn't be found, offset - %d", offset);
            return *it->second;
        }


    private:
        static int Traverse(PyObject *object, visitproc visit, void *arg) {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
            return 0;
        }

        static void Dealloc(PyObject *object) {
            //No need to call reference forget - being called by _Py_Dealloc
            PyTypeObject *type = Py_TYPE(object);

            if (PyType_IS_GC(type))
                PyObject_GC_UnTrack(object);

            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_DECREF(type);

            ((Type *) (object + 1))->~Type();
            type->tp_free(object);
        }

        static int SetAttribute(PyObject *object, PyObject *attrName, PyObject *value) {
            PyTypeObject *type = CPyModuleContainer::Instance().GetType(CPyModuleContainer::TypeHash<hash_type>());
            CPYTHON_VERIFY(type != nullptr, "was unable to locate type");
            CPYTHON_VERIFY(attrName->ob_type == &PyUnicode_Type, "attrName must be py string type");
            object_ptr bytesObject(PyUnicode_AsASCIIString(attrName), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
            char *name = PyBytes_AsString(bytesObject.get());
            MembersDefs defs(type->tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs) {
                return strcmp(rhs.name, name) == 0;
            });
            if (it == defs.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Requested attribute - %s, was not found", name);

            self &cpythonClassType = static_cast<self &>(*type);
            IMemberAccessor &accessor = cpythonClassType.GetAccessor(it->offset);
            accessor.Set(object, value);
            return 0;
        }

        static PyObject* GetAttribute(PyObject *object, PyObject *attrName)
        {
            PyTypeObject *type = CPyModuleContainer::Instance().GetType(CPyModuleContainer::TypeHash<hash_type>());
            CPYTHON_VERIFY(type != nullptr, "was unable to locate type");
            CPYTHON_VERIFY(attrName->ob_type == &PyUnicode_Type, "attrName must be py unicode type");
            object_ptr bytesObject(PyUnicode_AsASCIIString(attrName), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
            char *name = PyBytes_AsString(bytesObject.get());
            MembersDefs defs(type->tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs) {
                return strcmp(rhs.name, name) == 0;
            });
            if (it == defs.end())
            {
                Dictionary dictionary(object);
                object_ptr descriptor = dictionary.GetObject(attrName);
                if(descriptor.get() != nullptr && descriptor->ob_type == &PyMethodDescr_Type)
                    return GetMethod(descriptor, object);
                else
                    return  PyObject_GenericGetAttr(object, attrName);
            }

            self &cpythonClassType = static_cast<self &>(*type);
            IMemberAccessor &accessor = cpythonClassType.GetAccessor(it->offset);
            return accessor.Get(object);
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

    private:
        std::unordered_map<int, std::shared_ptr<IMemberAccessor>> m_membersAccessors; //Nothing is shared, but due to the fact vptr is not allowed we will use raii to keep the memory in check.
    };
}
