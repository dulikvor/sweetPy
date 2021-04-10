#pragma once

#include <frameobject.h>
#include "../Types/ObjectPtr.h"
#include "Deleter.h"

namespace sweetPy {
    class Stack
    {
    public:
        static ObjectPtr GetObject(size_t index)
        {
            PyThreadState* ts = PyThreadState_Get();
            ObjectPtr object(*(ts->frame->f_valuestack + index), &Deleter::Borrow);
            return object;
        }
    };
}
