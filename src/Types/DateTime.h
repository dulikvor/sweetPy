#pragma once

#include <chrono>
#include <Python.h>
#include <datetime.h>
#include "Core/Deleter.h"

namespace sweetPy{

class DateTime
{
public:
    DateTime();
    explicit DateTime(int hours, int minutes, int seconds, int milliseconds, int microseconds);
    DateTime(PyObject* dateTime);
    DateTime(const object_ptr& dateTime);
    bool operator==(const DateTime& rhs) const;

    std::chrono::microseconds GetDuration() const;
    PyObject* ToPython();
    static inline __attribute__((always_inline)) void ImportDateTimeModule()
    {
        if(PyDateTimeAPI == nullptr) //No need to use fences thanks to GIL
            PyDateTime_IMPORT;
    }

private:
    using HOURS = std::chrono::hours;
    using MINUTES = std::chrono::minutes;
    using SECONDS = std::chrono::seconds;
    using MILLISECONDS = std::chrono::milliseconds;
    using MICROSECONDS = std::chrono::microseconds;

private:
    std::chrono::microseconds m_duration;
};

}