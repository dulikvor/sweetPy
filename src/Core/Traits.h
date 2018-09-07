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
    template<typename T, typename std::enable_if<std::__and_<
            std::__not_<std::is_array<T>>, std::__not_<std::is_function<T>>>::value, bool>::type = true>
    struct base{
        /*typedef typename std::remove_const<typename std::remove_reference<T>::type>::type NonConst;
        typedef typename std::conditional<std::is_lvalue_reference<T>::value, typename std::add_lvalue_reference<NonConst>::type,
                typename std::conditional<std::is_rvalue_reference<T>::value, typename std::add_rvalue_reference<NonConst>::type,
                NonConst>::type>::type Type;*/
        typedef T Type;
    };


    template<>
    struct base<const char*>{
        typedef const char* Type;
    };

    template<typename T> struct is_container : public std::false_type{};
    template<typename T> struct is_container<std::vector<T>> : public std::true_type{};
}
