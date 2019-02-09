#pragma once

#include <Python.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <functional>
#include <type_traits>
#include <iterator>
#include <cstdint>
#include <bits/unordered_map.h>
#include "core/Param.h"
#include "../Core/Deleter.h"


namespace sweetPy{
    class Tuple
    {
    private:
        typedef std::vector<std::unique_ptr<core::Param>> Elements;
        class Converter
        {
        public:
            typedef std::function<PyObject*(void const * const)> ConveterFunc;
            Converter(void const* value, const ConveterFunc& converter);
            operator PyObject*() const;
        private:
            void const * const m_value;
            ConveterFunc m_converter;
        };
    public:
        Tuple();
        Tuple(PyObject* tuple);
        Tuple(const Tuple& obj);
        Tuple& operator=(const Tuple& rhs);
        Tuple(Tuple&& obj);
        Tuple& operator=(Tuple&& rhs);
        bool operator==(const Tuple& rhs) const;
        bool operator!=(const Tuple& rhs) const{ return operator==(rhs) == false; }
        void Clear();
        template<typename T, typename X = typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, object_ptr>::value &&
                !std::is_pointer<typename std::remove_reference<T>::type>::value &&
                !std::is_same<typename std::remove_reference<T>::type, std::nullptr_t>::value>::type>
        void AddElement(T&& element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            if(core::IsTypeIdExists<X, ARGUMENTS>::value == false)
            {
                if(std::is_rvalue_reference<T>::value)
                    throw core::Exception(__CORE_SOURCE, "rvalue will cause a dangling pointer scenario.");
                void const * ptr = &element;
                m_elements.emplace_back(new core::TypedParam<void*>(ptr));
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(ptr, converterFunc));
            }
            else
                m_elements.emplace_back(new core::TypedParam<X>(std::forward<T>(element)));
        }
        template<int N>
        void AddElement(const char (&element)[N])
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(std::move(_element)));
        }
        void AddElement(char* element)
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(_element));
        }
        void AddElement(const char* element)
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(_element));
        }
        void AddElement(void* element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
            
            if(element)
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(element, converterFunc));
        }
        void AddElement(const void* element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
            
            if(element)
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(element, converterFunc));
        }
        void AddElement(const std::nullptr_t& element)
        {
            m_elements.emplace_back(new core::TypedParam<void*>((void*)element));
        }
        void AddElement(const object_ptr& element);
        template<typename T, typename = typename std::enable_if<!std::is_pointer<T>::value>::type>
        const T& GetElement(size_t index) const
        {
            if(m_elements.size() < index)
                throw core::Exception(__CORE_SOURCE, "index exceeds number of elements");
            return static_cast<core::TypedParam<T>&>(*m_elements[index]).template Get<T>();
        }
        template<typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
        T GetElement(size_t index) const
        {
            if(m_elements.size() < index)
                throw core::Exception(__CORE_SOURCE, "index exceeds number of elements");
            return static_cast<core::TypedParam<T>&>(*m_elements[index]).template Get<T>();
        }
        PyObject* ToPython() const;
    
        class iterator : public std::iterator<std::forward_iterator_tag, core::Param* const, std::ptrdiff_t, core::Param* const, core::Param&>
        {
        public:
            explicit iterator(Elements::iterator it): m_it(it){}
            iterator& operator++(){m_it++; return *this;}
            bool operator==(const iterator& other) const{return m_it == other.m_it;}
            bool operator!=(const iterator& other) const{return m_it != other.m_it;}
            reference operator*() const{return **m_it;}
            pointer operator->() const{return m_it->get();}
    
        private:
            Elements::iterator m_it;
        };
    
        class const_iterator : public std::iterator<std::forward_iterator_tag, core::Param* const, std::ptrdiff_t, core::Param* const, const core::Param&>
        {
        public:
            explicit const_iterator(Elements::const_iterator it): m_it(it){}
            const const_iterator& operator++(){m_it++; return *this;}
            bool operator==(const const_iterator& other) const{return m_it == other.m_it;}
            bool operator!=(const const_iterator& other) const{return m_it != other.m_it;}
            reference operator*() const{return **m_it;}
            pointer operator->() const{return m_it->get();}
    
        private:
            Elements::const_iterator m_it;
        };
    
        class reverse_iterator : public std::iterator<std::forward_iterator_tag, core::Param* const, std::ptrdiff_t, core::Param* const, core::Param&>
        {
        public:
            explicit reverse_iterator(Elements::reverse_iterator it): m_it(it){}
            reverse_iterator& operator++(){m_it++; return *this;}
            bool operator==(const reverse_iterator& other) const{return m_it == other.m_it;}
            bool operator!=(const reverse_iterator& other) const{return m_it != other.m_it;}
            reference operator*() const{return **m_it;}
            pointer operator->() const{return m_it->get();}
    
        private:
            Elements::reverse_iterator m_it;
        };
    
        class const_reverse_iterator : public std::iterator<std::forward_iterator_tag, core::Param* const, std::ptrdiff_t, core::Param* const, const core::Param&>
        {
        public:
            explicit const_reverse_iterator(Elements::const_reverse_iterator it): m_it(it){}
            const const_reverse_iterator& operator++(){m_it++; return *this;}
            bool operator==(const const_reverse_iterator& other) const{return m_it == other.m_it;}
            bool operator!=(const const_reverse_iterator& other) const{return m_it != other.m_it;}
            reference operator*() const{return **m_it;}
            pointer operator->() const{return m_it->get();}
    
        private:
            Elements::const_reverse_iterator m_it;
        };
    
        iterator begin(){return iterator(m_elements.begin());}
        iterator end(){return iterator(m_elements.end());}
        const_iterator begin()const{return const_iterator(m_elements.begin());}
        const_iterator end() const{return const_iterator(m_elements.end());}
        reverse_iterator rbegin(){return reverse_iterator(m_elements.rbegin());}
        reverse_iterator rend(){return reverse_iterator(m_elements.rend());}
        const_reverse_iterator rbegin()const{return const_reverse_iterator(m_elements.rbegin());}
        const_reverse_iterator rend() const{return const_reverse_iterator(m_elements.rend());}

    private:
        std::unordered_map<size_t, Converter> m_converters;
        Elements m_elements;
    };
}
