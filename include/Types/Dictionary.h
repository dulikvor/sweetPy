#include <Python.h>
#include <unordered_map>
#include <type_traits>
#include <core/Exception.h>
#include "../Core/Deleter.h"
#include "../Core/Assert.h"
#include "../Core/Exception.h"
#include "../Core/Traits.h"

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
        
        template<typename Key, typename Value>
        void add(Key&& key, Value&& value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Value>
        void add(const char* key, Value&& value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<Value>::to_python(std::forward<Value>(value)), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        template<typename Key>
        void add(Key&& key, const char* value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<Key>::to_python(std::forward<Key>(key)), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<const char*>::to_python(value), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
    
        void add(const char* key, const char* value)
        {
            ObjectPtr pyKey = ObjectPtr(Object<const char*>::to_python(key), &Deleter::Owner);
            ObjectPtr pyValue = ObjectPtr(Object<const char*>::to_python(value), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(-1 != PyDict_SetItem(m_dict.get(), pyKey.release(), pyValue.release()));
        }
        
        template<typename Value, typename Key>
        Value get(Key&& key) const
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
            template<typename Value, typename = enable_if_t<std::is_copy_constructible<Value>::value>>
            Value get() const
            {
                return Object<Value>::from_python(m_value.get());
            }
            template<typename Value, typename = enable_if_t<std::is_copy_constructible<Value>::value>>
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
            
            iterator& operator++()
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
