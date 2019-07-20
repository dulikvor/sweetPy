#pragma once

#include <Python.h>
#include <bitset>
#include "../Core/Traits.h"
#include "../Core/Utils.h"
#include "CPythonType.h"
#include "TypesContainer.h"


namespace sweetPy {
    template<typename T>
    struct ClazzObject
    {
#define PROPERTIE_SIZE 1
        enum Propertie
        {
            Value = 0,
            Reference = 1,
            All = 2
        };
        constexpr ClazzObject()
        {
            static_assert(sizeof(PyObject) >= sizeof(std::bitset<Propertie::All>), "Symbol set is not packed after PyObject due to aligment");
        }
        constexpr static std::size_t get_size()
        {
            return sizeof(ClazzObject);
        }
        static void set_propertie(void* ptr, Propertie propertie)
        {
            auto objectPtr = reinterpret_cast<ClazzObject*>(ptr);
            objectPtr->m_propertie[propertie] = true;
        }
        static bool is_ref(void* ptr)
        {
            auto objectPtr = reinterpret_cast<ClazzObject*>(ptr);
            return objectPtr->m_propertie[Reference] == true;
        }
        static T& get_val(void* ptr)
        {
            return reinterpret_cast<ClazzObject*>(ptr)->m_val;
        }
        static const T& get_const_val(void* ptr)
        {
            return reinterpret_cast<ClazzObject*>(ptr)->m_val;
        }
        static char* get_val_offset(void* ptr)
        {
            return (char*)&reinterpret_cast<ClazzObject*>(ptr)->m_val;
        }
    
    private:
        PyObject m_object;
        std::bitset<PROPERTIE_SIZE> m_propertie;
        T m_val;
    };
    
    template<typename T, typename _T = remove_reference_t <T>>
    class ValueObject {
    public:
        template<typename X = _T, typename = enable_if_t<std::is_copy_constructible<X>::value>>
        static PyObject *alloc(const _T &object)
        {
            CPythonType& type = TypesContainer::instance().get_type(Hash::generate_hash_code<_T>());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
            new(ClazzObject<_T>::get_val_offset(ptr))_T(object);
            ClazzObject<_T>::set_propertie(ptr, ClazzObject<_T>::Propertie::Value);
            return ptr;
        }
        template<typename X = _T, typename = enable_if_t<!std::is_copy_constructible<X>::value &&
                std::is_move_constructible<X>::value>
                >
        static PyObject *alloc(_T &&object)
        {
            CPythonType& type = TypesContainer::instance().get_type(Hash::generate_hash_code<_T>());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
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
            CPythonType& type = TypesContainer::instance().get_type(Hash::generate_hash_code<Self>());
            PyObject *ptr = type.ht_type.tp_alloc(&type.ht_type, 0);
            new(ClazzObject<Self>::get_val_offset(ptr))Self(object);
            ClazzObject<Self>::set_propertie(ptr, ClazzObject<Self>::Reference);
            return ptr;
        }
    
    private:
        _T &m_ref;
    };
}
