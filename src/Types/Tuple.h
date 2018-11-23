#pragma once

#include <Python.h>
#include <vector>
#include <memory>
#include <type_traits>
#include "core/Param.h"
#include "../Core/Deleter.h"


namespace sweetPy{
    class Tuple
    {
    public:
        Tuple();
        Tuple(PyObject* tuple);
        Tuple(const Tuple& obj);
        Tuple& operator=(const Tuple& rhs);
        Tuple(Tuple&& obj);
        Tuple& operator=(Tuple&& rhs);
        void Clear();
        template<typename T, typename X = typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, object_ptr>::value &&
                !std::is_pointer<typename std::remove_reference<T>::type>::value &&
                !std::is_same<typename std::remove_reference<T>::type, std::nullptr_t>::value>::type>
        void AddElement(size_t index, T&& element)
        {
            m_elements.emplace_back(new core::TypedParam<X>(std::forward<T>(element)));
        }
        template<int N>
        void AddElement(size_t index, const char (&element)[N])
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(std::move(_element)));
        }
        void AddElement(size_t index, char* element)
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(_element));
        }
        void AddElement(size_t index, const char* element)
        {
            std::string _element = element;
            m_elements.emplace_back(new core::TypedParam<std::string>(_element));
        }
        void AddElement(size_t index, void* element)
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
        }
        void AddElement(size_t index, const void* element)
        {
            m_elements.emplace_back(new core::TypedParam<void*>(element));
        }
        void AddElement(size_t index, const std::nullptr_t& element)
        {
            m_elements.emplace_back(new core::TypedParam<void*>((void*)element));
        }
        void AddElement(size_t index, const object_ptr& element);
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

    private:
        std::vector<std::unique_ptr<core::Param>> m_elements;
    };
}
