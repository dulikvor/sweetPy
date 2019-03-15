#pragma once

#include <Python.h>
#include "Container.h"

namespace sweetPy{
    class Tuple : public _Container
    {
    public:
        Tuple() = default;
        ~Tuple() NOEXCEPT(true) override = default;
        Tuple(PyObject* tuple);
        Tuple(const Tuple& obj);
        Tuple& operator=(const Tuple& rhs);
        Tuple(Tuple&& obj);
        Tuple& operator=(Tuple&& rhs);
        bool operator==(const Tuple& rhs) const;
        bool operator!=(const Tuple& rhs) const{ return operator==(rhs) == false; }
        PyObject* ToPython() const;
    };
}
