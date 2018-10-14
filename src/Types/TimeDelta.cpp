#include "TimeDelta.h"
#include "../Core/Assert.h"

namespace sweetPy{

    TimeDelta::TimeDelta():m_duration(0) {}

    TimeDelta::TimeDelta(int seconds, int milliseconds, int microseconds)
    {
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MILLISECONDS(milliseconds)) +
                                  MICROSECONDS(microseconds)
        );
    }

    TimeDelta::TimeDelta(PyObject* timeDelta)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDelta_Check(timeDelta) != 0, "Provided object type must match datetime.datetime");
        int seconds = PyDateTime_DELTA_GET_SECONDS(timeDelta);
        int microSec = PyDateTime_DELTA_GET_MICROSECONDS(timeDelta);
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }

    TimeDelta::TimeDelta(const object_ptr &timeDelta)
    {
        ImportDateTimeModule();
        CPYTHON_VERIFY(PyDelta_Check(timeDelta.get()) != 0, "Provided object type must match datetime.datetime");
        int seconds = PyDateTime_DELTA_GET_SECONDS(timeDelta.get());
        int microSec = PyDateTime_DELTA_GET_MICROSECONDS(timeDelta.get());
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  MICROSECONDS(microSec)
        );
    }


    bool TimeDelta::operator==(const TimeDelta& rhs) const
    {
        return m_duration == rhs.m_duration;
    }

    std::chrono::microseconds TimeDelta::GetDuration() const
    {
        return m_duration;
    }

    PyObject* TimeDelta::ToPython()
    {
        ImportDateTimeModule();
        auto seconds = std::chrono::duration_cast<SECONDS>(m_duration);
        auto microseconds = std::chrono::duration_cast<MICROSECONDS>(m_duration) - std::chrono::duration_cast<MICROSECONDS>(seconds);
        return PyDelta_FromDSU(0, seconds.count(), microseconds.count());
    }
}

