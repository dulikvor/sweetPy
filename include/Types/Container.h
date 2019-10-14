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
#include "core/NoExcept.h"
#include "core/Exception.h"
#include "core/Param.h"
#include "ObjectPtr.h"
#include "../Core/Traits.h"

namespace sweetPy{
    class Tuple;
    class List;
    
    class _Container
    {
    private:
        friend class Tuple;
        friend class List;
        typedef std::vector<std::unique_ptr<core::Param>> Elements;
        class Converter
        {
        public:
            typedef std::function<PyObject*(void* const)> ConveterFunc;
            Converter(const ConveterFunc &converter): m_converter(converter) {}
            PyObject* operator ()(void* const ptr) const
            {
                return m_converter(ptr);
            }
        private:
            ConveterFunc m_converter;
        };
    
        _Container() = default;
        virtual ~_Container() NOEXCEPT(true) = default;
        _Container(const _Container &obj)
                :m_converters(obj.m_converters)
        {
            for(auto& element : obj.m_elements)
            {
                core::Param::Param_Ptr param;
                if(element->IsInt())
                {
                    core::TypedParam<int>& typedElement =  static_cast<core::TypedParam<int>&>(*element);
                    param.reset(new core::TypedParam<int>(typedElement));
                }
                else if(element->IsBool())
                {
                    core::TypedParam<bool>& typedElement =  static_cast<core::TypedParam<bool>&>(*element);
                    param.reset(new core::TypedParam<bool>(typedElement));
                }
                else if(element->IsDouble())
                {
                    core::TypedParam<double>& typedElement =  static_cast<core::TypedParam<double>&>(*element);
                    param.reset(new core::TypedParam<double>(typedElement));
                }
                else if(element->IsCtypeS())
                {
                    core::TypedParam<char*>& typedElement =  static_cast<core::TypedParam<char*>&>(*element);
                    param.reset(new core::TypedParam<char*>(typedElement));
                }
                else if(element->IsString())
                {
                    core::TypedParam<std::string>& typedElement =  static_cast<core::TypedParam<std::string>&>(*element);
                    param.reset(new core::TypedParam<std::string>(typedElement));
                }
                else if(element->IsPointer())
                {
                    core::TypedParam<void *> &typedElement = static_cast<core::TypedParam<void *> &>(*element);
                    param.reset(new core::TypedParam<void *>(typedElement));
                }
                else
                    param = element->Clone();
            
                m_elements.emplace_back(std::move(param));
            }
        }
        _Container& operator=(const _Container& rhs)
        {
            for(auto& element : rhs.m_elements)
            {
                core::Param::Param_Ptr param;
                if(element->IsInt())
                {
                    core::TypedParam<int>& typedElement =  static_cast<core::TypedParam<int>&>(*element);
                    param.reset(new core::TypedParam<int>(typedElement));
                }
                else if(element->IsBool())
                {
                    core::TypedParam<bool>& typedElement =  static_cast<core::TypedParam<bool>&>(*element);
                    param.reset(new core::TypedParam<bool>(typedElement));
                }
                else if(element->IsDouble())
                {
                    core::TypedParam<double>& typedElement =  static_cast<core::TypedParam<double>&>(*element);
                    param.reset(new core::TypedParam<double>(typedElement));
                }
                else if(element->IsCtypeS())
                {
                    core::TypedParam<char*>& typedElement =  static_cast<core::TypedParam<char*>&>(*element);
                    param.reset(new core::TypedParam<char*>(typedElement));
                }
                else if(element->IsString())
                {
                    core::TypedParam<std::string>& typedElement =  static_cast<core::TypedParam<std::string>&>(*element);
                    param.reset(new core::TypedParam<std::string>(typedElement));
                }
                else if(element->IsPointer())
                {
                    core::TypedParam<void *> &typedElement = static_cast<core::TypedParam<void *> &>(*element);
                    param.reset(new core::TypedParam<void *>(typedElement));
                }
                else
                    param = element->Clone();
        
                m_elements.emplace_back(std::move(param));
            }
            m_converters = rhs.m_converters;
    
            return *this;
        }
        _Container(_Container &&obj) NOEXCEPT(true) : m_elements(std::move(obj.m_elements)){}
        _Container& operator=(_Container &&rhs) NOEXCEPT(true){m_elements = std::move(rhs.m_elements); return *this;}
        bool operator==(const _Container& rhs) const
        {
            return std::equal(begin(), end(), rhs.begin());
        }
        bool operator!=(const _Container& rhs) const{ return operator==(rhs) == false; }
        
    public:
        void clear()
        {
            m_elements.clear();
        }
        template<typename T, typename X = typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                typename = enable_if_t<!std::is_same<typename std::remove_reference<T>::type, ObjectPtr>::value &&
                                                   !std::is_pointer<typename std::remove_reference<T>::type>::value &&
                                                   !std::is_same<typename std::remove_reference<T>::type, std::nullptr_t>::value>>
        void add_element(T&& element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            if(core::IsTypeIdExists<X, ARGUMENTS>::value == false)
            {
                m_elements.emplace_back(new core::TypedParam<X>(std::forward<T>(element)));
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(converterFunc));
            }
            else
                m_elements.emplace_back(new core::TypedParam<X>(std::forward<T>(element)));
        }
        template<int N>
        void add_element(const char (&element)[N])
        {
            m_elements.emplace_back(new core::TypedParam<char*>(element));
        }
        void add_element(char* element)
        {
            m_elements.emplace_back(new core::TypedParam<char*>(element));
        }
        void add_element(const char* element)
        {
            m_elements.emplace_back(new core::TypedParam<char*>(element));
        }
        void add_element(void* element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
        
            if(element)
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(converterFunc));
        }
        void add_element(const void* element, const Converter::ConveterFunc& converterFunc = Converter::ConveterFunc())
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
        
            if(element)
                m_converters.emplace(std::piecewise_construct, std::make_tuple(m_elements.size() - 1), std::make_tuple(converterFunc));
        }
        void add_element(const std::nullptr_t& element)
        {
            m_elements.emplace_back(new core::TypedParam<void*>((void*)element));
        }
        void add_element(const ObjectPtr& element);
        template<typename T, typename = typename std::enable_if<!std::is_pointer<T>::value>::type>
        const T& get_element(size_t index) const
        {
            if(m_elements.size() < index)
                throw core::Exception(__CORE_SOURCE, "index exceeds number of elements");
            return static_cast<core::TypedParam<T>&>(*m_elements[index]).template Get<T>();
        }
        template<typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
        T get_element(size_t index) const
        {
            if(m_elements.size() < index)
                throw core::Exception(__CORE_SOURCE, "index exceeds number of elements");
            return static_cast<core::TypedParam<T>&>(*m_elements[index]).template Get<T>();
        }
        
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