#pragma once

#include <Python.h>
#include "Core/Traits.h"
#include "TypesContainer.h"
#include "src/Detail/ClazzPyType.h"
#include "src/Detail/Object.h"
#include "Module.h"

namespace sweetPy {
    
    template<typename T, typename _T = remove_reference_t<T>>
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
            m_context = context;
        }
        ~ReferenceType()
        {
            CPythonType& type = *CPythonType::get_type(m_type.get());
            PyType_Ready(&type.ht_type);
            type.clear_trace_ref();
            TypesContainer::instance().add_type(type.get_hash_code(), type, true);
            m_module.add_type(type.get_hash_code(), std::move(m_type));
        }

    private:
        static void free_type(PyObject* ptr)
        {
            delete static_cast<PyType*>(CPythonType::get_type(ptr));
        }
        
    private:
        Module& m_module;
        ObjectPtr m_type;
        ClazzContext& m_context;
    };
}