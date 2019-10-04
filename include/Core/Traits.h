#pragma once

#include <type_traits>
#include <vector>
#include <tuple>

namespace sweetPy{
    template<typename X, typename... Args>
    struct py_type_id{};
    
    template<typename X>
    struct py_type_id<X>
    {
        const static int value = -1;
    };
    
    
    template<typename X, typename... Args>
    struct py_type_id<X, X, Args...>
    {
        const static int value = 0;
    };
    
    template<typename X, typename T, typename... Args>
    struct py_type_id<X, T, Args...>
    {
        const static int value = py_type_id<X, Args...>::value != -1 ? py_type_id<X, Args...>::value + 1 : -1;
    };
    
    template<typename T>
    using decay_t = typename std::decay<T>::type;
    template<bool val, typename T = void>
    using enable_if_t = typename std::enable_if<val, T>::type;
    template<typename T>
    using remove_reference_t = typename std::remove_reference<T>::type;
    
    template<typename... Args>
    struct filter_result{};
    template<typename T, typename... Args>
    struct filter_result<T, std::tuple<Args...>>
    {
        using type = std::tuple<T, Args...>;
    };
    
    template <typename...> struct filter{};
    template <typename Predicator> struct filter<Predicator> { using type = std::tuple<>; };
    template<typename Predicator, typename T, typename... Args>
    struct filter<Predicator, T, Args...>
    {
        using type = typename std::conditional<Predicator::template get_value<T>(),
                typename filter_result<T, typename filter<Predicator, Args...>::type>::type,
                typename filter<Predicator, Args...>::type
                >::type;
        
    };
    
    template<typename Predict, bool Expected = true>
    struct Predicator
    {
        template<typename T>
        constexpr static bool get_value()
        {
            using predict = typename Predict::template rebind<T>::type;
            return predict::value == Expected;
        }
    };
    
    template<typename T = void>
    struct is_reference_predicator : public std::is_reference<T>
    {
        template<typename X>
        struct rebind
        {
            typedef is_reference_predicator<X> type;
        };
    };
    
    template<typename T, typename _T = typename std::remove_pointer<T>::type>
    struct is_function_pointer : public std::is_function<_T>{};
    
    template<typename... Args>
    void invoker(Args&&...){}
    
    template<typename T> struct is_container : public std::false_type{};
    template<typename T> struct is_container<std::vector<T>> : public std::true_type{};
}
