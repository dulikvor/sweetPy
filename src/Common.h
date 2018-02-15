#pragma once

#include <functional>
#include <type_traits>
#include <Python.h>
#include "Lock.h"
#include "Exception.h"

namespace pycppconn{
#define CPYTHON_VERIFY(expression, reason) do{ if(!(expression)) throw CPythonException(PyExc_StandardError, SOURCE, reason); }while(0)

//Partial version of decay, functions and array are excluded
template<typename T, typename std::enable_if<std::__and_<
        std::__not_<std::is_array<T>>, std::__not_<std::is_function<T>>>::value, bool>::type = true>
struct base{
    typedef typename std::remove_const<typename std::remove_reference<T>::type>::type Type;
};


template<>
struct base<const char*>{
    typedef const char* Type;
};
}