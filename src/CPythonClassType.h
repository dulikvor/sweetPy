#pragma once

#include <string>
#include <Python.h>
#include "CPythonType.h"
#include "IMemberAccessor.h"
#include "Exception.h"


namespace sweetPy {

    template<typename Type>
    class CPythonClassType : public CPythonType {
    public:
        typedef CPythonClassType<Type> self;

        CPythonClassType(const std::string &name, const std::string &doc)
                : CPythonType(name, doc) {
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
            PyTypeObject *type = CPyModuleContainer::Instance().GetType(CPyModuleContainer::TypeHash<self>());
            CPYTHON_VERIFY(type != nullptr, "was unable to locate type");
            CPYTHON_VERIFY(attrName->ob_type == &PyString_Type, "attrName must be py string type");
            char *name = PyString_AsString(attrName);
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

        static PyObject* GetAttribute(PyObject *object, PyObject *attrName) {
            PyTypeObject *type = CPyModuleContainer::Instance().GetType(CPyModuleContainer::TypeHash<self>());
            CPYTHON_VERIFY(type != nullptr, "was unable to locate type");
            CPYTHON_VERIFY(attrName->ob_type == &PyString_Type, "attrName must be py string type");
            char *name = PyString_AsString(attrName);
            MembersDefs defs(type->tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs) {
                return strcmp(rhs.name, name) == 0;
            });
            if (it == defs.end())
               return  PyObject_GenericGetAttr(object, attrName);

            self &cpythonClassType = static_cast<self &>(*type);
            IMemberAccessor &accessor = cpythonClassType.GetAccessor(it->offset);
            return accessor.Get(object);
        }


    private:
        std::unordered_map<int, std::shared_ptr<IMemberAccessor>> m_membersAccessors; //Nothing is shared, but due to the fact vptr is not allowed we will use raii to keep the memory in check.
    };
}
