#pragma once

#include <chrono>
#include <Python.h>
#include <datetime.h>
#include "Core/Deleter.h"

namespace sweetPy {

class TimeDelta
{
public:
    TimeDelta();
    explicit TimeDelta(int seconds, int milliseconds, int microseconds);
    TimeDelta(PyObject* timeDelta);
    TimeDelta(const object_ptr& timeDelta);
    bool operator==(const TimeDelta& rhs) const;

    std::chrono::microseconds GetDuration() const;
    PyObject* ToPython();
    static inline __attribute__((always_inline)) void ImportDateTimeModule()
    {
        if(PyDateTimeAPI == nullptr) //No need to use fences thanks to GIL
            PyDateTime_IMPORT;
    }

private:
    using SECONDS = std::chrono::seconds;
    using MILLISECONDS = std::chrono::milliseconds;
    using MICROSECONDS = std::chrono::microseconds;

private:
    std::chrono::microseconds m_duration;
};

}
