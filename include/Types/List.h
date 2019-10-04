#pragma once

#include <Python.h>
#include "core/NoExcept.h"
#include "../Core/Deleter.h"
#include "../Detail/CPythonObject.h"
#include "Container.h"
#include "ObjectPtr.h"

namespace sweetPy{
    class List: public _Container
    {
    public:
        List() = default;
        ~List() NOEXCEPT(true) override = default;
        explicit List(PyObject* list)
        {
            size_t size = PyList_Size(list);
            for(int idx = 0; idx < size; idx++)
                add_element(ObjectPtr(PyList_GetItem(list, idx), &Deleter::Borrow));
        }
        List(const List& obj)
                :_Container(obj){}
        List& operator=(const List& rhs)
        {
            static_cast<_Container&>(*this) = static_cast<const _Container&>(rhs);
            return *this;
        }
        List(List &&obj) NOEXCEPT(true) : _Container(std::move(static_cast<_Container&>(obj))){}
        List& operator=(List&& rhs) NOEXCEPT(true)
        {
            static_cast<_Container&>(*this) = std::move(static_cast<_Container&>(rhs));
            return *this;
        }
        bool operator==(const List& rhs) const
        {
            return static_cast<const _Container&>(*this) == static_cast<const _Container&>(rhs);
        }
        bool operator!=(const List& rhs) const{ return operator==(rhs) == false; }
        PyObject* to_python() const
        {
            ObjectPtr list(PyList_New(m_elements.size()), &Deleter::Owner);
            for(int idx = 0; idx < m_elements.size(); idx++)
            {
                PyObject* object = nullptr;
                if(m_elements[idx]->IsInt())
                    object = Object<int>::to_python(static_cast<core::TypedParam<int>&>(*m_elements[idx]).Get<int>());
                else if(m_elements[idx]->IsString())
                    object = Object<std::string>::to_python(static_cast<core::TypedParam<std::string>&>(*m_elements[idx]).Get<std::string>());
                else if(m_elements[idx]->IsDouble())
                    object = Object<double>::to_python(static_cast<core::TypedParam<double>&>(*m_elements[idx]).Get<double>());
                else if(m_elements[idx]->IsBool())
                    object = Object<bool>::to_python(static_cast<core::TypedParam<bool>&>(*m_elements[idx]).Get<bool>());
                else
                {
                    auto it = m_converters.find(idx);
                    if(it != m_converters.end())
                    {
                        object = it->second(m_elements[idx].get()->GetBuffer());
                    }
                    else
                    {
                        Py_XINCREF(Py_None);
                        object = Py_None;
                    }
                }
                PyList_SetItem(list.get(), idx, object);
            }
            return list.release();
        }
    };
    
    template<>
    struct Object<List>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef List Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        
        static List get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyList_Check(object))
            {
                new(toBuffer)List(object);
                return *reinterpret_cast<List*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<List>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<List>& refObject = ClazzObject<ReferenceObject<List>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const List>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const List>& refObject = ClazzObject<ReferenceObject<const List>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "List can only originates from tuple object and ref List object");
        }
        static List from_python(PyObject* object)
        {
            GilLock lock;
            if(PyList_Check(object))
            {
                return List(object);
            }
            else if(ClazzObject<ReferenceObject<List>>::is_ref(object))
            {
                ReferenceObject<List>& refObject = ClazzObject<ReferenceObject<List>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const List>>::is_ref(object))
            {
                ReferenceObject<const List>& refObject = ClazzObject<ReferenceObject<const List>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "List can only originates from tuple object and ref List object");
        }
        static PyObject* to_python(const List& value)
        {
            return value.to_python();
        }
    };
    
    template<>
    struct Object<const List&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef List Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        
        static const List& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyList_Check(object))
            {
                new(toBuffer)List(object);
                return *reinterpret_cast<List*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<const List>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const List>& refObject = ClazzObject<ReferenceObject<const List>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "List can only originates from ref const List type or tuple object");
        }
        static const List& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const List>>::is_ref(object))
            {
                ReferenceObject<const List>& refObject = ClazzObject<ReferenceObject<const List>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "List can only originates from ref const List type");
        }
        static PyObject* to_python(const List& value)
        {
            return ReferenceObject<const List>::alloc(value);
        }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<const List&, I>
    {
        typedef typename Object<const List&>::FromPythonType FromPythonType;
        typedef typename Object<const List&>::Type Type;
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

