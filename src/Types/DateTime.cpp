#include "DateTime.h"
#include "../Core/Assert.h"

namespace sweetPy{

    DateTime::DateTime():m_duration(0) {}

    DateTime::DateTime(int hours, int minutes, int seconds, int milliseconds, int microseconds)
    {
        m_duration = MICROSECONDS(std::chrono::duration_cast<MICROSECONDS>(HOURS(hours)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MINUTES(minutes)) +
                                  std::chrono::duration_cast<MICROSECONDS>(SECONDS(seconds)) +
                                  std::chrono::duration_cast<MICROSECONDS>(MILLISECONDS(milliseconds)) +
                                  MICROSECONDS(microseconds)
        );
    }

    DateTime::DateTime(PyObject* dateTime)
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

    DateTime::DateTime(const object_ptr &dateTime)
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


    bool DateTime::operator==(const DateTime& rhs) const
    {
       return m_duration == rhs.m_duration;
    }

    std::chrono::microseconds DateTime::GetDuration() const
    {
        return m_duration;
    }

    PyObject* DateTime::ToPython()
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
}
