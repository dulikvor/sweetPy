#pragma once

#include <Python.h>
#include "Container.h"

namespace sweetPy{
    class List: public _Container
    {
    public:
        List() = default;
        ~List() NOEXCEPT(true) override = default;
        List(PyObject* list);
        List(const List& obj);
        List& operator=(const List& rhs);
        List(List&& obj) NOEXCEPT(true);
        List& operator=(List&& rhs) NOEXCEPT(true);
        bool operator==(const List& rhs) const;
        bool operator!=(const List& rhs) const{ return operator==(rhs) == false; }
        PyObject* ToPython() const;
    };
}

