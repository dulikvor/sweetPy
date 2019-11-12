#pragma once

#include <Python.h>
#include <unordered_map>
#include <type_traits>
#include <core/Exception.h>
#include "../Detail/DictionaryElement.h"
#include "../Core/Deleter.h"
#include "../Core/Assert.h"
#include "../Core/Exception.h"
#include "../Core/Traits.h"
#include "AsciiString.h"
#include "List.h"
#include "Tuple.h"

namespace sweetPy{
    class Dictionary
    {
    private:
        struct end_pos{};
        
    public:
        struct key_by_ref_t{};
        struct value_by_ref_t{};
        struct key_value_by_ref_t{};
        typedef Detail::ElementValue Key;
        typedef Detail::ElementValue Value;
        typedef std::pair<Key, Value> KeyValuePair;
        
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
        
        Dictionary(const Dictionary& object)
                :m_dict(nullptr, &Deleter::Owner)
        {
            m_dict.reset(PyDict_Copy(object.m_dict.get()));
            CPYTHON_VERIFY_EXC(m_dict.get());
        }
        
        Dictionary& operator=(const Dictionary& rhs)
        {
            m_dict.reset(PyDict_Copy(rhs.m_dict.get()));
            CPYTHON_VERIFY_EXC(m_dict.get());
            return *this;
        }
        
        bool operator==(const Dictionary& rhs) const
        {
            bool isEqual = true;
            for(auto& kvp : rhs)
            {
                if(!isEqual)
                    return isEqual;
                auto key = kvp.first.get<PyObject*>();
                if(PyDict_Contains(m_dict.get(), key) == false)
                    return false;
                auto value = get<PyObject*>(key);
                auto rhsValue = kvp.second.get<PyObject*>();
                if(value->ob_type != rhsValue->ob_type)
                    return false;
                
    
                if(PyLong_CheckExact(value) ||
                   ClazzObject<ReferenceObject<int>>::is_ref(value) ||
                   ClazzObject<ReferenceObject<const int>>::is_ref(value))
                {
                    isEqual =  Object<int>::from_python(value) == Object<int>::from_python(rhsValue);
                }
                else if(PyFloat_CheckExact(value) ||
                        ClazzObject<ReferenceObject<double>>::is_ref(value) ||
                        ClazzObject<ReferenceObject<const double>>::is_ref(value))
                {
                    isEqual = Object<double>::from_python(value) == Object<double>::from_python(rhsValue);
                }
                else if(PyBool_Check(value))
                {
                    isEqual = Object<bool>::from_python(value) == Object<bool>::from_python(rhsValue);
                }
                else if(PyBytes_CheckExact(value) ||
                        ClazzObject<ReferenceObject<std::string>>::is_ref(value) ||
                        ClazzObject<ReferenceObject<const std::string>>::is_ref(value))
                {
                    isEqual = Object<std::string>::from_python(value) == Object<std::string>::from_python(rhsValue);
                }
                else if(PyUnicode_CheckExact(value))
                {
                    isEqual = Object<std::string>::from_python(value) == Object<std::string>::from_python(rhsValue);
                }
                else if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(value) ||
                        ClazzObject<ReferenceObject<const AsciiString>>::is_ref(value))
                {
                    isEqual = Object<AsciiString>::from_python(value) == Object<AsciiString>::from_python(rhsValue);
                }
                else if(PyList_CheckExact(value) ||
                        ClazzObject<ReferenceObject<List>>::is_ref(value) ||
                        ClazzObject<ReferenceObject<const List>>::is_ref(value))
                {
                    isEqual = Object<List>::from_python(value) == Object<List>::from_python(rhsValue);
                }
                else if(PyTuple_CheckExact(value) ||
                        ClazzObject<ReferenceObject<Tuple>>::is_ref(value) ||
                        ClazzObject<ReferenceObject<const Tuple>>::is_ref(value))
                {
                    isEqual = Object<Tuple>::from_python(value) == Object<Tuple>::from_python(rhsValue);
                }
                else
                    return false;
            }
            return isEqual;
        }
        
        bool operator!=(const Dictionary& rhs) const
        {
            return operator==(rhs) == false;
        }
        
        template<typename Key, typename Value, typename KeyNoRef = remove_reference_t<Key>, typename ValueNoRef = remove_reference_t<Value>>
        void add(Key&& key, Value&& value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<KeyNoRef>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<ValueNoRef>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key, typename Value, typename ValueNoRef = remove_reference_t<Value>>
        void add(Key&& key, Value&& value, key_by_ref_t)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<ValueNoRef>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key, typename Value, typename KeyNoRef = remove_reference_t<Key>>
        void add(Key&& key, Value&& value, value_by_ref_t)
        {
            ObjectPtr pyKey = ObjectPtr(Object<KeyNoRef>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key, typename Value>
        void add(Key&& key, Value&& value, key_value_by_ref_t)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Value, typename ValueNoRef = remove_reference_t<Value>>
        void add(const char* key, Value&& value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<ValueNoRef>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key, typename KeyNoRef = remove_reference_t<Key>>
        void add(Key&& key, const char* value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<KeyNoRef>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<const char*>::to_python(value), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key>
        void add(Key&& key, const char* value, key_by_ref_t)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<const char*>::to_python(value), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Value>
        void add(const char* key, Value&& value, value_by_ref_t)
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        void add(const char* key, const char* value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<const char*>::to_python(value), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Value, typename Key, typename KeyNoRef = remove_reference_t<Key>>
        Value get(Key&& key) const
        {
            ObjectPtr pyKey = ObjectPtr(Object<KeyNoRef>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            auto object = ObjectPtr(PyDict_GetItem(m_dict.get(), pyKey.get()), &Deleter::Borrow);
            if(object.get() == nullptr)
                throw core::Exception(__CORE_SOURCE, "key was not found");
            return Object<Value>::from_python(object.get());
        }
    
        template<typename Value, typename Key>
        Value get(Key&& key, key_by_ref_t) const
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            auto object = ObjectPtr(PyDict_GetItem(m_dict.get(), pyKey.get()), &Deleter::Borrow);
            if(object.get() == nullptr)
                throw core::Exception(__CORE_SOURCE, "key was not found");
            return Object<Value>::from_python(object.get());
        }
    
        template<typename Value>
        Value get(const char* key) const
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            auto object = ObjectPtr(PyDict_GetItem(m_dict.get(), pyKey.get()), &Deleter::Borrow);
            if(object.get() == nullptr)
                throw core::Exception(__CORE_SOURCE, "key was not found");
            return Object<Value>::from_python(object.get());
        }
        
        void clear()
        {
            PyDict_Clear(m_dict.get());
        }
        
        PyObject* to_python() const
        {
            Py_INCREF(m_dict.get());
            return m_dict.get();
        }
        
        class iterator : public std::iterator<std::forward_iterator_tag, KeyValuePair, ptrdiff_t, KeyValuePair*, KeyValuePair&>
        {
        public:
            iterator(PyObject* const dict, end_pos): m_dict(dict), m_pos(0)
            {
                CPYTHON_VERIFY(dict != nullptr, "Provided dictionary == null");
                m_count = PyDict_Size(dict);
                CPYTHON_VERIFY_EXC(m_count > -1);
            }
    
            iterator(PyObject* const dict): m_dict(dict), m_pos(0), m_count(0)
            {
                CPYTHON_VERIFY(dict != nullptr, "Provided dictionary == null");
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_value = std::make_pair(ObjectPtr(key, &Deleter::Borrow), ObjectPtr(value, &Deleter::Borrow));
            }
            
            iterator& operator++()
            {
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_count++;
                m_value = std::make_pair(ObjectPtr(key, &Deleter::Borrow), ObjectPtr(value, &Deleter::Borrow));
                return *this;
            }
            
            bool operator==(const iterator& other) const{return m_count == other.m_count;}
            bool operator!=(const iterator& other) const{return m_count != other.m_count;}
            reference operator*() {return m_value;}
            pointer operator->() {return &m_value;}
    
        private:
            PyObject* const m_dict;
            Py_ssize_t m_pos;
            Py_ssize_t m_count;
            KeyValuePair m_value;
        };
    
        class const_iterator : public std::iterator<std::forward_iterator_tag, KeyValuePair, ptrdiff_t, KeyValuePair* const, const KeyValuePair&>
        {
        public:
            const_iterator(PyObject* const dict, end_pos): m_dict(dict), m_pos(0)
            {
                CPYTHON_VERIFY(dict != nullptr, "Provided dictionary == null");
                m_count = PyDict_Size(dict);
                CPYTHON_VERIFY_EXC(m_count > -1);
            }
        
            const_iterator(PyObject* const dict): m_dict(dict), m_pos(0), m_count(0)
            {
                CPYTHON_VERIFY(dict != nullptr, "Provided dictionary == null");
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_value = std::make_pair(ObjectPtr(key, &Deleter::Borrow), ObjectPtr(value, &Deleter::Borrow));
            }
        
            const_iterator& operator++()
            {
                PyObject *key, *value;
                PyDict_Next(const_cast<PyObject *>(m_dict), &m_pos, &key, &value);
                m_count++;
                m_value = std::make_pair(ObjectPtr(key, &Deleter::Borrow), ObjectPtr(value, &Deleter::Borrow));
                return *this;
            }
        
            bool operator==(const const_iterator& other) const{return m_count == other.m_count;}
            bool operator!=(const const_iterator& other) const{return m_count != other.m_count;}
            reference operator*() {return m_value;}
            pointer operator->() {return &m_value;}
    
        private:
            PyObject* const m_dict;
            Py_ssize_t m_pos;
            Py_ssize_t m_count;
            KeyValuePair m_value;
        };
    
        iterator begin(){return iterator(m_dict.get());}
        iterator end(){return iterator(m_dict.get(), end_pos{});}
        const_iterator begin() const{return const_iterator(m_dict.get());}
        const_iterator end() const{return const_iterator(m_dict.get(), end_pos{});}
    
    private:
        ObjectPtr m_dict;
    };
    
    template<>
    struct Object<Dictionary>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef Dictionary Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        
        static Dictionary get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyDict_Check(object))
            {
                new(toBuffer)Dictionary(object);
                return *reinterpret_cast<Dictionary*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<Dictionary>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<Dictionary>& refObject = ClazzObject<ReferenceObject<Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Dictionary>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const Dictionary>& refObject = ClazzObject<ReferenceObject<const Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Dictionary can only originates from dict object and ref Dictionary object");
        }
        static Dictionary from_python(PyObject* object)
        {
            GilLock lock;
            if(PyDict_Check(object))
            {
                return Dictionary(object);
            }
            else if(ClazzObject<ReferenceObject<Dictionary>>::is_ref(object))
            {
                ReferenceObject<Dictionary>& refObject = ClazzObject<ReferenceObject<Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Dictionary>>::is_ref(object))
            {
                ReferenceObject<const Dictionary>& refObject = ClazzObject<ReferenceObject<const Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Dictionary can only originates from dict object and ref Dictionary object");
        }
        static PyObject* to_python(const Dictionary& value)
        {
            return value.to_python();
        }
    };
    
    template<>
    struct Object<const Dictionary&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef Dictionary Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        
        static const Dictionary& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyDict_Check(object))
            {
                new(toBuffer)Dictionary(object);
                return *reinterpret_cast<Dictionary*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<const Dictionary>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const Dictionary>& refObject = ClazzObject<ReferenceObject<const Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Dictionary can only originates from ref const Dictionary type or dict object");
        }
        
        static const Dictionary& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const Dictionary>>::is_ref(object))
            {
                ReferenceObject<const Dictionary>& refObject = ClazzObject<ReferenceObject<const Dictionary>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Dictionary can only originates from ref const Dictionary type");
        }
        
        static PyObject* to_python(const Dictionary& value)
        {
            return ReferenceObject<const Dictionary>::alloc(value);
        }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<const Dictionary&, I>
    {
        typedef typename Object<const Dictionary&>::FromPythonType FromPythonType;
        typedef typename Object<const Dictionary&>::Type Type;
        static void* destructor(char* buffer)
        {
            if(*reinterpret_cast<std::uint32_t*>(buffer) != MAGIC_WORD)
            {
                Type* typedPtr = reinterpret_cast<Type*>(buffer);
                typedPtr->~Type();
            }
            return nullptr;
        }
    };
}
