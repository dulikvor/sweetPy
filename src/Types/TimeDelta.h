#pragma once

#include <chrono>
#include <Python.h>
#include <datetime.h>
#include "src/Detail/CPythonObject.h"
#include "../Core/Assert.h"
#include "ObjectPtr.h"

namespace sweetPy {

class TimeDelta
{
private:
    using SECONDS = std::chrono::seconds;
    using MILLISECONDS = std::chrono::milliseconds;
    using MICROSECONDS = std::chrono::microseconds;
    
public:
    TimeDelta(): m_duration(0) {}
    TimeDelta(int seconds, int milliseconds, int microseconds)
    {
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MILLISECONDS(milliseconds)) +
                                  MICROSECONDS(microseconds)
        );
    }
    explicit TimeDelta(PyObject* timeDelta)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDelta_Check(timeDelta) != 0, "Provided object type must match datetime.datetime");
        int seconds = PyDateTime_DELTA_GET_SECONDS(timeDelta);
        int microSec = PyDateTime_DELTA_GET_MICROSECONDS(timeDelta);
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }
    explicit TimeDelta(const ObjectPtr& timeDelta)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDelta_Check(timeDelta.get()) != 0, "Provided object type must match datetime.datetime");
        int seconds = PyDateTime_DELTA_GET_SECONDS(timeDelta.get());
        int microSec = PyDateTime_DELTA_GET_MICROSECONDS(timeDelta.get());
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }
    bool operator==(const TimeDelta& rhs) const
    {
        return m_duration == rhs.m_duration;
    }
    std::chrono::microseconds get_duration() const{ return m_duration; }
    PyObject* to_python() const
    {
        ImportDateTimeModule();
        auto seconds = std::chrono::duration_cast<SECONDS>(m_duration);
        auto microseconds = std::chrono::duration_cast<MICROSECONDS>(m_duration) - std::chrono::duration_cast<MICROSECONDS>(seconds);
        return PyDelta_FromDSU(0, seconds.count(), microseconds.count());
    }
    static inline __attribute__((always_inline)) void ImportDateTimeModule()
    {
        if(PyDateTimeAPI == nullptr) //No need to use fences thanks to GIL
            PyDateTime_IMPORT;
    }
    
private:
    std::chrono::microseconds m_duration;
};

template<>
struct Object<TimeDelta>
{
public:
    typedef PyObject* FromPythonType;
    typedef TimeDelta Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    static TimeDelta get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        TimeDelta::ImportDateTimeModule();
        if(PyDelta_CheckExact(object))
        {
            new(toBuffer)TimeDelta(object);
            return *reinterpret_cast<TimeDelta*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<TimeDelta>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<TimeDelta>& refObject = ClazzObject<ReferenceObject<TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const TimeDelta>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const TimeDelta>& refObject = ClazzObject<ReferenceObject<const TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "TimeDelta can only originates from ref const TimeDelta type or datetime.timedelta object");
    }
    static TimeDelta from_python(PyObject* object)
    {
        GilLock lock;
        TimeDelta::ImportDateTimeModule();
        if(PyDelta_CheckExact(object))
        {
            return TimeDelta(object);
        }
        else if(ClazzObject<ReferenceObject<TimeDelta>>::is_ref(object))
        {
            ReferenceObject<TimeDelta>& refObject = ClazzObject<ReferenceObject<TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const TimeDelta>>::is_ref(object))
        {
            ReferenceObject<const TimeDelta>& refObject = ClazzObject<ReferenceObject<const TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "TimeDelta can only originates from ref const TimeDelta type or datetime.timedelta object");
    }
    
    static PyObject* to_python(const TimeDelta& value)
    {
        return value.to_python();
    }
};

template<>
struct Object<const TimeDelta&>
{
public:
    typedef PyObject* FromPythonType;
    typedef TimeDelta Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    
    static const TimeDelta& get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        TimeDelta::ImportDateTimeModule();
        if(PyDelta_CheckExact(object))
        {
            new(toBuffer)TimeDelta(object);
            return *reinterpret_cast<TimeDelta*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<const TimeDelta>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const TimeDelta>& refObject = ClazzObject<ReferenceObject<const TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "TimeDelta can only originates from ref const TimeDelta type or datetime.timedelta object");
    }
    static const TimeDelta& from_python(PyObject* object)
    {
        GilLock lock;
        if(ClazzObject<ReferenceObject<const TimeDelta>>::is_ref(object))
        {
            ReferenceObject<const TimeDelta>& refObject = ClazzObject<ReferenceObject<const TimeDelta>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "TimeDelta can only originates from ref const TimeDelta type");
    }
    static PyObject* to_python(const TimeDelta& value)
    {
        return ReferenceObject<const TimeDelta>::alloc(value);
    }
};

template<std::size_t I>
struct ObjectWrapper<const TimeDelta&, I>
{
    typedef typename Object<const TimeDelta&>::FromPythonType FromPythonType;
    typedef typename Object<const TimeDelta&>::Type Type;
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
