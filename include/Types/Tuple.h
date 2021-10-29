#pragma once

#include <Python.h>
#include "../Detail/CPythonObject.h"
#include "Container.h"
#include "ObjectPtr.h"

namespace sweetPy{
    class Tuple : public _Container
    {
    public:
        Tuple() = default;
        ~Tuple() NOEXCEPT(true) override = default;
        explicit Tuple(PyObject* tuple)
        {
            std::size_t size = PyTuple_Size(tuple);
            for(int idx = 0; idx < size; idx++)
                add_element(ObjectPtr(PyTuple_GetItem(tuple, idx), &Deleter::Borrow));
        }
        Tuple(const Tuple& obj):_Container(obj){}
        Tuple& operator=(const Tuple& rhs)
        {
            static_cast<_Container&>(*this) = static_cast<const _Container&>(rhs);
            return *this;
        }
        explicit Tuple(Tuple &&obj): _Container(std::move(static_cast<_Container&>(obj))){}
        Tuple& operator=(Tuple&& rhs)
        {
            static_cast<_Container&>(*this) = std::move(static_cast<_Container&>(rhs));
            return *this;
        }
        bool operator==(const Tuple& rhs) const
        {
            return static_cast<const _Container&>(*this) == static_cast<const _Container&>(rhs);
        }
        bool operator!=(const Tuple& rhs) const{ return operator==(rhs) == false; }
        PyObject* to_python() const;
    };
    
    template<>
    struct Object<Tuple>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef Tuple Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;

        static Tuple get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyTuple_Check(object))
            {
                new(toBuffer)Tuple(object);
                return *reinterpret_cast<Tuple*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<Tuple>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<Tuple>& refObject = ClazzObject<ReferenceObject<Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Tuple>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const Tuple>& refObject = ClazzObject<ReferenceObject<const Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Tuple can only originates from tuple object and ref Tuple object");
        }
        static Tuple from_python(PyObject* object)
        {
            GilLock lock;
            if(PyTuple_Check(object))
            {
                return Tuple(object);
            }
            else if(ClazzObject<ReferenceObject<Tuple>>::is_ref(object))
            {
                ReferenceObject<Tuple>& refObject = ClazzObject<ReferenceObject<Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else if(ClazzObject<ReferenceObject<const Tuple>>::is_ref(object))
            {
                ReferenceObject<const Tuple>& refObject = ClazzObject<ReferenceObject<const Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Tuple can only originates from tuple object and ref Tuple object");
        }
        static PyObject* to_python(const Tuple& value)
        {
            return value.to_python();
        }
    };

    template<>
    struct Object<const Tuple&>
    {
    public:
        typedef PyObject* FromPythonType;
        typedef Tuple Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;

        static const Tuple& get_typed(char* fromBuffer, char* toBuffer)
        {
            static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
            PyObject* object = *(PyObject**)fromBuffer;
            if(PyTuple_Check(object))
            {
                new(toBuffer)Tuple(object);
                return *reinterpret_cast<Tuple*>(toBuffer);
            }
            else if(ClazzObject<ReferenceObject<const Tuple>>::is_ref(object))
            {
                new(toBuffer)std::uint32_t(MAGIC_WORD);
                ReferenceObject<const Tuple>& refObject = ClazzObject<ReferenceObject<const Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Tuple can only originates from ref const Tuple type or tuple object");
        }

        static const Tuple& from_python(PyObject* object)
        {
            GilLock lock;
            if(ClazzObject<ReferenceObject<const Tuple>>::is_ref(object))
            {
                ReferenceObject<const Tuple>& refObject = ClazzObject<ReferenceObject<const Tuple>>::get_val(object);
                return refObject.get_ref();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Tuple can only originates from ref const Tuple type");
        }

        static PyObject* to_python(const Tuple& value)
        {
            return ReferenceObject<const Tuple>::alloc(value);
        }
    };
    
    template<std::size_t I>
    struct ObjectWrapper<const Tuple&, I>
    {
        typedef typename Object<const Tuple&>::FromPythonType FromPythonType;
        typedef typename Object<const Tuple&>::Type Type;
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
