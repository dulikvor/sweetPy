#pragma once

#include <Python.h>
#include <vector>
#include <memory>
#include <structmember.h>
#include "Types/ObjectPtr.h"
#include "Core/Deleter.h"
#include "src/Detail/ClazzContext.h"
#include "src/Detail/CPythonType.h"
#include "src/Detail/Function.h"
#include <iostream>

namespace sweetPy {
    class MetaClass : public CPythonType
    {
    public:
        typedef std::unique_ptr<ClazzContext> ClazzContextPtr;
        typedef std::unique_ptr<Function> FunctionPtr;
        MetaClass(const std::string& name, const std::string& doc)
            : CPythonType(name, doc, 0, &meta_free), m_context(new ClazzContext())
        {
            ht_type.ob_base.ob_base.ob_type = &PyType_Type;
            ht_type.ob_base.ob_base.ob_refcnt = 2;
            //ob_size was re defined in order to identify meta class instances in runtime, no allocation is allowed, so the original usage is absolute.
            ht_type.ob_base.ob_size = get_check_sum();
            //Allocating and initializing a new instance by using the meta class is blocked.
            ht_type.tp_new = nullptr;
            ht_type.tp_alloc = nullptr;
            ht_type.tp_init = nullptr;
            ht_type.tp_name = m_name.c_str();
            ht_type.tp_basicsize = (Py_ssize_t)(sizeof(PyHeapTypeObject));
            ht_type.tp_doc = m_doc.c_str();
            ht_type.tp_is_gc = &is_collectable;
            ht_type.tp_base = &PyType_Type;
            ht_type.tp_free = &free_type;
        }
        ClazzContext& get_context(){ return *m_context; }
        void finalize()
        {
            PyType_Ready(&ht_type);
            clear_trace_ref();
            init_static_methods();
        }
        static MetaClass& get_common_meta_type(){ return m_commonMetaType; }
        static Py_ssize_t get_check_sum(){ return 0xABCCBA; }
        void add_static_method(FunctionPtr&& function)
        {
            m_staticMethods.emplace_back(std::move(function));
        }

    private:
        static int is_collectable(PyObject *obj)
        {
            return static_cast<int>(false);
        }
        void init_static_methods()
        {
            if(m_staticMethods.empty() == false)
            {
                for(auto& method : m_staticMethods)
                {
                    auto hashCodeVal = method->get_hash_code();
                    auto descriptor = method->to_python();
                    ObjectPtr name(PyUnicode_FromString(descriptor->ml_name), &Deleter::Owner);
                    ObjectPtr hash_code(PyLong_FromUnsignedLong(hashCodeVal), &Deleter::Owner);
    
                    ObjectPtr self(PyTuple_New(2), &Deleter::Owner); //self = name and object, function will release the tuple
    
                    ObjectPtr type(PyCapsule_New(this, nullptr,[](PyObject*){}), &Deleter::Owner);
                    PyTuple_SetItem(self.get(), 0, type.release());
                    PyTuple_SetItem(self.get(), 1, hash_code.release());
                    
                    ObjectPtr cFunction(PyCFunction_NewEx(descriptor.release(), self.release(), NULL), &Deleter::Owner);
                    PyDict_SetItem(ht_type.tp_dict, name.get(), cFunction.get());
    
                    m_context->add_member_function(hashCodeVal, std::move(method));
                }
            }
        }
        //static void Dealloc(PyObject *object);
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REFS
            if(ob_base.ob_base._ob_prev)
                ob_base.ob_base._ob_prev->_ob_next = ob_base.ob_base._ob_next;
            if(ob_base.ob_base._ob_next)
                ob_base.ob_base._ob_next->_ob_prev = ob_base.ob_base._ob_prev;
    
            ob_base.ob_base._ob_next = NULL;
            ob_base.ob_base._ob_prev = NULL;
#endif
        }
        static void free_type(void* ptr)
        {
            auto type = static_cast<CPythonType*>(reinterpret_cast<PyHeapTypeObject*>(ptr));
            type->get_free()(type);
        }
        static void meta_free(void* ptr)
        {
            reinterpret_cast<MetaClass*>(ptr)->~MetaClass();
        }
        
    private:
        static MetaClass m_commonMetaType;
        ClazzContextPtr m_context;
        typedef std::vector<FunctionPtr> StaticMethods;
        StaticMethods m_staticMethods;
    };
}


