#pragma once

#include <Python.h>
#include <functional>
#include "CPythonType.h"
#include "MetaClass.h"

namespace sweetPy{
    class PlainType : public CPythonType
    {
    public:
        typedef std::function<void(PyObject*)> DeallocObject;
        PlainType(const std::string &name, const std::string &doc, Py_ssize_t size, std::size_t hash_code, const DeallocObject& dealloc)
                : CPythonType(name, doc, hash_code, std::forward<DeallocObject>(free_type)), m_dealloc(dealloc)
        {
            ht_type.ob_base.ob_base.ob_type = &MetaClass::get_common_meta_type().ht_type;
            ht_type.ob_base.ob_base.ob_refcnt = 2;
            ht_type.ob_base.ob_size = 0;
            ht_type.tp_name = m_name.c_str();
            ht_type.tp_basicsize = size;
            ht_type.tp_dealloc = &dealloc_object;
            ht_type.tp_flags = Py_TPFLAGS_HEAPTYPE;
            ht_type.tp_setattro = &set_attribute;
            ht_type.tp_new = PyBaseObject_Type.tp_new;
        }
        bool is_finalized() const { return m_isFinalized; }
        const DeallocObject& get_dealloc() const {return m_dealloc;}
        void finalize()
        {
            if(m_isFinalized)
                throw CPythonException(PyExc_Exception, __CORE_SOURCE, "PlainType is already finalized");
            auto docBuf = (char*)malloc(sizeof(char)*(m_doc.size() + 1));
            std::copy(m_doc.begin(), m_doc.end(), docBuf);
            docBuf[m_doc.size()] = '\0';
            ht_type.tp_doc = docBuf;
            
            PyType_Ready(&ht_type);
            clear_trace_ref();
            m_isFinalized = true;
        }
        
    private:
        static void dealloc_object(PyObject* object)
        {
            auto type = reinterpret_cast<PyHeapTypeObject*>(Py_TYPE(object));
            auto plainType = static_cast<PlainType*>(type);
            plainType->get_dealloc()(object);
            PyBaseObject_Type.tp_dealloc(object);
        }
        static void free_type(PyObject* ptr)
        {
            delete static_cast<PlainType*>(CPythonType::get_type(ptr));
        }
        static int set_attribute(PyObject *object, PyObject *attrName, PyObject *value)
        {
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Operation is not supported");
            return 0;
        }
        
    private:
        DeallocObject m_dealloc;
        bool m_isFinalized = false;
    };
}