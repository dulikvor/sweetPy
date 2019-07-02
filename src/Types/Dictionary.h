#include <Python.h>
#include <unordered_map>
#include <core/Exception.h>
#include "../Core/Deleter.h"
#include "../Core/Assert.h"
#include "../Core/Exception.h"

namespace sweetPy{
    class Dictionary
    {
    public:
        Dictionary()
            :m_dict(PyDict_New(), &Deleter::Owner)
        {
        }
        explicit Dictionary(PyObject* dictionary)
            :m_dict(nullptr, &Deleter::Owner)
        {
            Py_INCREF(dictionary);
            m_dict.reset(dictionary);
        }
        ~Dictionary() noexcept = default;
        template<typename Key, typename Value>
        void add(Key&& key, Value&& value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.get(), pyValue.get()));
        }
        template<typename Value, typename Key>
        Value get(const Key& key) const
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(key), &Deleter::Owner);
            auto object = ObjectPtr(PyDict_GetItem(m_dict.get(), pyKey.get()), &Deleter::Borrow);
            if(object.get() == nullptr)
                throw core::Exception(__CORE_SOURCE, "key was not found");
            return Object<Value>::from_python(object.get());
        }
        class ElementValue
        {
        public:
            ElementValue() = default;
            explicit ElementValue(ObjectPtr&& value)
                :m_value(std::move(value)){}
            ElementValue& operator=(ObjectPtr&& value)
            {
                m_value = std::move(value);
                return *this;
            }
            template<typename Value>
            Value get() const
            {
                return Object<Value>::from_python(m_value.get());
            }
            template<typename Value>
            operator Value()
            {
                return Object<Value>::from_python(m_value.get());
            }
        private:
            ObjectPtr m_value;
        };
        class iterator : public std::iterator<std::forward_iterator_tag, ElementValue, ptrdiff_t, ElementValue*, ElementValue&>
        {
        public:
            explicit iterator(PyObject* const dict): m_dict(dict)
            {
                m_pos = PyDict_Size(const_cast<PyObject*>(dict));
            }
            iterator(PyObject* const dict, Py_ssize_t pos): m_dict(dict), m_pos(pos)
            {
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_value = ObjectPtr(value, &Deleter::Borrow);
            }
            iterator& operator++(int)
            {
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_value = ObjectPtr(value, &Deleter::Borrow);
                return *this;
            }
            bool operator==(const iterator& other) const{return m_pos == other.m_pos;}
            bool operator!=(const iterator& other) const{return m_pos != other.m_pos;}
            reference operator*() {return m_value;}
            pointer operator->() {return &m_value;}
    
        private:
            PyObject* const m_dict;
            Py_ssize_t m_pos;
            ElementValue m_value;
        };
    
        iterator begin(){return iterator(m_dict.get(), 0);}
        iterator end(){return iterator(m_dict.get());}
    
    private:
        ObjectPtr m_dict;
    };
}
