#pragma once

#include <Python.h>
#include <memory>
#include <functional>
#include <type_traits>
#include <vector>
#include "core/Source.h"
#include "Lock.h"
#include "Exception.h"
#include "src/Core/Deleter.h"

namespace sweetPy{


    //Will not accept array, function pointers types, will remove const from - T const/T const&, will retain the value category.
    template<typename T, typename std::enable_if<!std::is_function<T>::value, bool>::type = true>
    struct base{
        typedef T Type;
    };

    template<typename T> struct is_container : public std::false_type{};
    template<typename T> struct is_container<std::vector<T>> : public std::true_type{};
}
