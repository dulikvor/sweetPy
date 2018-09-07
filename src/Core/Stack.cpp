#include "Stack.h"
#include <frameobject.h>

namespace sweetPy{
    object_ptr Stack::GetObject(size_t index)
    {
        PyThreadState* ts = PyThreadState_Get();
        object_ptr object(*(ts->frame->f_valuestack + index), &Deleter::Borrow);
        return std::move(object);
    }
}
