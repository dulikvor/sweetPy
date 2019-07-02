#pragma once

#include <Python.h>
#include "Core/Traits.h"
#include "TypesContainer.h"
#include "src/Detail/ClazzPyType.h"
#include "src/Detail/Object.h"
#include "Module.h"

namespace sweetPy {
    
    template<typename T, typename _T = decay_t<T>>
    class PlainReferenceType
    {
    private:
        typedef ReferenceObject<_T> ObjectType;
        typedef ClazzPyType<ObjectType> PyType;
        
    public:
        PlainReferenceType(Module& module, const std::string& name, const std::string& doc)
        : m_module(module),
          m_type(CPythonType::get_py_object(new ClazzPyType<ObjectType>(name, doc, &free_type)), &Deleter::Owner)
        {
            reinterpret_cast<PyTypeObject*>(m_type.get())->tp_dealloc = PyType_Type.tp_dealloc;
        }
        ~PlainReferenceType()
        {
            auto& type = *CPythonType::get_type(m_type.get());
            if(!m_module.is_type_exists(type.get_hash_code()))
            {
                PyType_Ready(&type.ht_type);
                clear_trace_ref();
                TypesContainer::instance().add_type(type.get_hash_code(), type, false);
                m_module.add_type(type.get_hash_code(), std::move(m_type), false);
            }
        }
        
    private:
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REFS
            auto type = (PyTypeObject*)m_type.get();
            if(type->ob_base.ob_base._ob_prev)
                type->ob_base.ob_base._ob_prev->_ob_next = type->ob_base.ob_base._ob_next;
            if(type->ob_base.ob_base._ob_next)
                type->ob_base.ob_base._ob_next->_ob_prev = type->ob_base.ob_base._ob_prev;
        
            type->ob_base.ob_base._ob_next = NULL;
            type->ob_base.ob_base._ob_prev = NULL;
#endif
        }
        static void free_type(void* ptr)
        {
            delete reinterpret_cast<PyType*>(ptr);
        }
        
    private:
        Module& m_module;
        ObjectPtr m_type;
    };
    
    template<typename T, typename _T = decay_t<T>>
    class ReferenceType
    {
    public:
        typedef ReferenceObject<_T> ObjectType;
        typedef ClazzPyType<ObjectType> PyType;
        ReferenceType(Module& module, const std::string& name, const std::string& doc, const ClazzContext& context)
        : m_module(module),
          m_type(CPythonType::get_py_object(new PyType(name, doc, &free_type)), &Deleter::Owner),
          m_context(static_cast<PyType*>(CPythonType::get_type(m_type.get()))->get_context())
        {
            reinterpret_cast<PyTypeObject*>(m_type.get())->tp_dealloc = PyType_Type.tp_dealloc;
            m_context = context;
        }
        ~ReferenceType()
        {
            auto& type = *CPythonType::get_type(m_type.get());
            PyType_Ready(&type.ht_type);
            clear_trace_ref();
            TypesContainer::instance().add_type(type.get_hash_code(), type, true);
            m_module.add_type(type.get_hash_code(), std::move(m_type));
        }

    private:
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REFS
            auto type = (PyTypeObject*)m_type.get();
            if(type->ob_base.ob_base._ob_prev)
                type->ob_base.ob_base._ob_prev->_ob_next = type->ob_base.ob_base._ob_next;
            if(type->ob_base.ob_base._ob_next)
                type->ob_base.ob_base._ob_next->_ob_prev = type->ob_base.ob_base._ob_prev;
        
            type->ob_base.ob_base._ob_next = NULL;
            type->ob_base.ob_base._ob_prev = NULL;
#endif
        }
        static void free_type(void* ptr)
        {
            delete reinterpret_cast<PyType*>(ptr);
        }
        
    private:
        Module& m_module;
        ObjectPtr m_type;
        ClazzContext& m_context;
    };
}