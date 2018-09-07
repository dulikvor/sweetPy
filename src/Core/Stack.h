#pragma once

#include "Deleter.h"

namespace sweetPy {
    class Stack
    {
    public:
        static object_ptr GetObject(size_t index);
    };
}
