#pragma once

#include <Python.h>
#include "Core/Traits.h"
#include "src/Detail/ClazzPyType.h"
#include "TypesContainer.h"


namespace sweetPy {
    template<typename T, typename _T = decay_t <T>>
    class ValueObject {
    public:
        template<typename X = _T, typename = enable_if_t<std::is_copy_constructible<X>::value>>
        static PyObject *alloc(const _T &object)
        {
            CPythonType& type = TypesContainer::instance().get_type(typeid(_T).hash_code());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
            std::cout<<"Address Object - "<<std::hex<<ptr<<std::endl;
            new(ClazzObject<_T>::get_val_offset(ptr))_T(object);
            ClazzObject<_T>::set_propertie(ptr, ClazzObject<_T>::Propertie::Value);
            return ptr;
        }
        template<typename X = _T, typename = enable_if_t<!std::is_copy_constructible<X>::value &&
                std::is_move_constructible<X>::value>
                >
        static PyObject *alloc(_T &&object)
        {
            CPythonType& type = TypesContainer::instance().get_type(typeid(_T).hash_code());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
            std::cout<<"Address Object - "<<std::hex<<ptr<<std::endl;
            new(ClazzObject<_T>::get_val_offset(ptr))_T(std::move(object));
            ClazzObject<_T>::set_propertie(ptr, ClazzObject<_T>::Propertie::Value);
            return ptr;
        }
    };
    
    template<typename T, typename _T = remove_reference_t<T>>
    class ReferenceObject {
    public:
        typedef ReferenceObject<_T> Self;
        
        ReferenceObject(_T &object) : m_ref(object) {}
        
        _T &get_ref() { return m_ref; }
        
        static PyObject *alloc(_T &object)
        {
            CPythonType& type = TypesContainer::instance().get_type(typeid(Self).hash_code());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
            std::cout<<"Address Object - "<<std::hex<<ptr<<std::endl;
            new(ClazzObject<Self>::get_val_offset(ptr))Self(object);
            ClazzObject<Self>::set_propertie(ptr, ClazzObject<Self>::Reference);
            return ptr;
        }
    
    private:
        _T &m_ref;
    };
}
