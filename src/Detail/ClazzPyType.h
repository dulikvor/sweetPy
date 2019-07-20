#pragma once

#include <Python.h>
#include <string>
#include "Core/Exception.h"
#include "Core/Assert.h"
#include "Core/Utils.h"
#include "MetaClass.h"
#include "CPythonType.h"
#include "Object.h"
#include "ClazzContext.h"
#include "MemberAccessor.h"

namespace sweetPy {
    
    template<typename T>
    class ClazzPyType : public CPythonType
    {
    public:
        typedef ClazzPyType<T> Self;
        typedef std::unique_ptr<MemberAccessor> MemeberAccessorPtr;
        typedef std::unique_ptr<ClazzContext> ClazzContextPtr;
        
        ClazzPyType(const std::string &name, const std::string &doc, Free freeType)
                : CPythonType(name, doc, Hash::generate_hash_code<T>(), freeType), m_context(new ClazzContext())
        {
            ht_type.ob_base.ob_base.ob_type = &MetaClass::get_common_meta_type().ht_type;
            ht_type.ob_base.ob_base.ob_refcnt = 2;
            ht_type.ob_base.ob_size = 0;
            ht_type.tp_name = m_name.c_str();
            ht_type.tp_basicsize = ClazzObject<T>::get_size();
            ht_type.tp_dealloc = &dealloc_object;
            ht_type.tp_flags = Py_TPFLAGS_HEAPTYPE;
            ht_type.tp_new = PyBaseObject_Type.tp_new;
            ht_type.tp_alloc = PyBaseObject_Type.tp_alloc;
            ht_type.tp_setattro = &set_attribute;
            ht_type.tp_getattro = &get_attribute;
    
            auto docBuf = (char*)malloc(sizeof(char)*(m_doc.size() + 1));
            std::copy(m_doc.begin(), m_doc.end(), docBuf);
            docBuf[m_doc.size()] = '\0';
            ht_type.tp_doc = docBuf;
        }
        ClazzPyType(const ClazzPyType&)=delete;
        ClazzPyType& operator=(const ClazzPyType&)=delete;
        ClazzContext& get_context(){ return *m_context; }
        
    private:
        static void dealloc_object(PyObject *object)
        {
            reinterpret_cast<T*>(ClazzObject<T>::get_val_offset(object))->~T();
            PyBaseObject_Type.tp_dealloc(object);
        }
        static int set_attribute(PyObject *object, PyObject *attrName, PyObject *value)
        {
            Self& type = *static_cast<Self*>(reinterpret_cast<PyHeapTypeObject*>(object->ob_type));
            CPYTHON_VERIFY(attrName->ob_type == &PyUnicode_Type, "attrName must be py string type");
            ObjectPtr bytesObject(PyUnicode_AsASCIIString(attrName), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
            char *name = PyBytes_AsString(bytesObject.get());
            MembersDefs defs(type.ht_type.tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs) {
                return strcmp(rhs.name, name) == 0;
            });
            if (it == defs.end())
                throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Requested attribute - %s, was not found", name);
            
            MemberAccessor &accessor = type.get_context().get_accessor(it->offset);
            accessor.set(object, value);
            return 0;
        }
        
        static PyObject* get_attribute(PyObject *object, PyObject *attrName)
        {
            Self& type = *static_cast<Self*>(reinterpret_cast<PyHeapTypeObject*>(object->ob_type));
            CPYTHON_VERIFY(attrName->ob_type == &PyUnicode_Type, "attrName must be py unicode type");
            ObjectPtr bytesObject(PyUnicode_AsASCIIString(attrName), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
            char *name = PyBytes_AsString(bytesObject.get());
            MembersDefs defs(type.ht_type.tp_members);
            auto it = std::find_if(defs.begin(), defs.end(), [&name](typename MembersDefs::iterator::reference rhs) {
                return strcmp(rhs.name, name) == 0;
            });
            if (it == defs.end())
            {
                ObjectPtr descriptor(PyDict_GetItem(type.ht_type.tp_dict, attrName), &Deleter::Borrow);
                if(descriptor.get() != nullptr && descriptor->ob_type == &PyMethodDescr_Type)
                    return get_method(descriptor, object);
                else
                    return  PyObject_GenericGetAttr(object, attrName);
            }
            
            MemberAccessor &accessor = type.get_context().get_accessor(it->offset);
            return accessor.get(object);
        }
        
        static PyObject* get_method(const ObjectPtr& descr, PyObject *obj)
        {
            PyMethodDescrObject& descriptor = static_cast<PyMethodDescrObject&>(*(PyMethodDescrObject*)descr.get());
            ObjectPtr self(PyTuple_New(2), &Deleter::Owner); //self = name and object, function will release the tuple
            Py_XINCREF(obj); //tuple steals the reference
            PyTuple_SetItem(self.get(), 0, obj);
            Py_XINCREF(descriptor.d_common.d_name); //tuple steals the reference
            PyTuple_SetItem(self.get(), 1, descriptor.d_common.d_name);
            return PyCFunction_NewEx(descriptor.d_method, self.get(), NULL);
        }
    
    private:
        ClazzContextPtr m_context;
    };
}
