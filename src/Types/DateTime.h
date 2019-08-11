#pragma once

#include <chrono>
#include <Python.h>
#include <datetime.h>
#include "src/Detail/CPythonObject.h"
#include "../Core/Assert.h"
#include "ObjectPtr.h"

namespace sweetPy{

class DateTime
{
private:
    using HOURS = std::chrono::hours;
    using MINUTES = std::chrono::minutes;
    using SECONDS = std::chrono::seconds;
    using MILLISECONDS = std::chrono::milliseconds;
    using MICROSECONDS = std::chrono::microseconds;
    
public:
    DateTime():m_duration(0) {}
    DateTime(int hours, int minutes, int seconds, int milliseconds, int microseconds)
    {
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(HOURS(hours)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MINUTES(minutes)) +
                                  std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MILLISECONDS(milliseconds)) +
                                  MICROSECONDS(microseconds)
        );
    }
    DateTime(PyObject* dateTime)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDateTime_Check(dateTime) != 0, "Provided object type must match datetime.datetime");
        int hours = PyDateTime_DATE_GET_HOUR(dateTime);
        int minutes = PyDateTime_DATE_GET_MINUTE(dateTime);
        int seconds = PyDateTime_DATE_GET_SECOND(dateTime);
        int microSec = PyDateTime_DATE_GET_MICROSECOND(dateTime);
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(HOURS(hours)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MINUTES(minutes)) +
                                  std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }
    explicit DateTime(const ObjectPtr& dateTime)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDateTime_Check(dateTime.get()) != 0, "Provided object type must match datetime.datetime");
        int hours = PyDateTime_DATE_GET_HOUR(dateTime.get());
        int minutes = PyDateTime_DATE_GET_MINUTE(dateTime.get());
        int seconds = PyDateTime_DATE_GET_SECOND(dateTime.get());
        int microSec = PyDateTime_DATE_GET_MICROSECOND(dateTime.get());
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(HOURS(hours)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MINUTES(minutes)) +
                                  std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }
    bool operator==(const DateTime& rhs) const
    {
        return m_duration == rhs.m_duration;
    }
    std::chrono::microseconds get_duration() const{ return m_duration; }
    PyObject* to_python() const
    {
        ImportDateTimeModule();
        auto hours = std::chrono::duration_cast<HOURS>(m_duration);
        auto minutes = std::chrono::duration_cast<MINUTES>(m_duration) - std::chrono::duration_cast<MINUTES>(hours);
        auto seconds = std::chrono::duration_cast<SECONDS>(m_duration) - std::chrono::duration_cast<SECONDS>(hours) - std::chrono::duration_cast<SECONDS>(minutes);
        auto milliseconds = std::chrono::duration_cast<MILLISECONDS>(m_duration) - std::chrono::duration_cast<MILLISECONDS>(hours) - std::chrono::duration_cast<MILLISECONDS>(minutes) - std::chrono::duration_cast<MILLISECONDS>(seconds);
        auto microseconds = std::chrono::duration_cast<MICROSECONDS>(m_duration) - std::chrono::duration_cast<MICROSECONDS>(hours) - std::chrono::duration_cast<MICROSECONDS>(minutes) - std::chrono::duration_cast<MICROSECONDS>(seconds) - std::chrono::duration_cast<MICROSECONDS>(milliseconds);
        return PyDateTime_FromDateAndTime(2018, 1, 1, hours.count(), minutes.count(), seconds.count(),
                                          std::chrono::duration_cast<MICROSECONDS >(milliseconds).count() + microseconds.count()
        );
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
struct Object<DateTime>
{
public:
    typedef PyObject* FromPythonType;
    typedef DateTime Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    static DateTime get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        DateTime::ImportDateTimeModule();
        if(PyDateTime_CheckExact(object))
        {
            new(toBuffer)DateTime(object);
            return *reinterpret_cast<DateTime*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<DateTime>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<DateTime>& refObject = ClazzObject<ReferenceObject<DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const DateTime>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const DateTime>& refObject = ClazzObject<ReferenceObject<const DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "DateTime can only originates from ref const DateTime type or datetime.datetime object");
    }
    static DateTime from_python(PyObject* object)
    {
        GilLock lock;
        DateTime::ImportDateTimeModule();
        if(PyDateTime_CheckExact(object))
        {
            return DateTime(object);
        }
        else if(ClazzObject<ReferenceObject<DateTime>>::is_ref(object))
        {
            ReferenceObject<DateTime>& refObject = ClazzObject<ReferenceObject<DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else if(ClazzObject<ReferenceObject<const DateTime>>::is_ref(object))
        {
            ReferenceObject<const DateTime>& refObject = ClazzObject<ReferenceObject<const DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "DateTime can only originates from ref const DateTime type or datetime.datetime object");
    }
    static PyObject* to_python(const DateTime& value)
    {
        return value.to_python();
    }
};

template<>
struct Object<const DateTime&>
{
public:
    typedef PyObject* FromPythonType;
    typedef DateTime Type;
    static constexpr const char *Format = "O";
    static const bool IsSimpleObjectType = false;
    static const DateTime& get_typed(char* fromBuffer, char* toBuffer)
    {
        static_assert(sizeof(Type) >= sizeof(std::uint32_t), "Not enough space to initialize magic word");
        PyObject* object = *(PyObject**)fromBuffer;
        DateTime::ImportDateTimeModule();
        if(PyDateTime_CheckExact(object))
        {
            new(toBuffer)DateTime(object);
            return *reinterpret_cast<DateTime*>(toBuffer);
        }
        else if(ClazzObject<ReferenceObject<const DateTime>>::is_ref(object))
        {
            new(toBuffer)std::uint32_t(MAGIC_WORD);
            ReferenceObject<const DateTime>& refObject = ClazzObject<ReferenceObject<const DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "DateTime can only originates from ref const DateTime type or datetime.datetime object");
    }
    static const DateTime& from_python(PyObject* object)
    {
        GilLock lock;
        if(ClazzObject<ReferenceObject<const DateTime>>::is_ref(object))
        {
            ReferenceObject<const DateTime>& refObject = ClazzObject<ReferenceObject<const DateTime>>::get_val(object);
            return refObject.get_ref();
        }
        else
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "DateTime can only originates from ref const DateTime type");
    }
    static PyObject* to_python(const DateTime& value)
    {
        return ReferenceObject<const DateTime>::alloc(value);
    }
};

template<std::size_t I>
struct ObjectWrapper<const DateTime&, I>
{
    typedef typename Object<const DateTime&>::FromPythonType FromPythonType;
    typedef typename Object<const DateTime&>::Type Type;
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
