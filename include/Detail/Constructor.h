#pragma once

#include <Python.h>
#include <type_traits>
#include <initializer_list>
#include <utility>
#include <string>
#include "../Core/Traits.h"
#include "ClazzPyType.h"
#include "CPythonObject.h"

namespace sweetPy {

    template<typename ClassType, typename... Args>
    class Constructor{
    public:
        Constructor(Constructor &) = delete;
        Constructor &operator=(Constructor &) = delete;
        Constructor(Constructor &&obj) = delete;
        Constructor &operator=(Constructor &&obj) = delete;

        template<std::size_t... I>
        static int wrapper_impl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<Args>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::FromPythonType...>::value)];
            char nativeArgsBuffer[std::max(1, ObjectsPackSize<typename Object<Args>::Type...>::value)];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,
                        ObjectWrapper<Args, I>...>::value)...), "Invalid argument was provided");
            }
            new(ClazzObject<ClassType>::get_val_offset(self))ClassType(std::forward<Args>(Object<Args>::get_typed(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<Args, I>,ObjectWrapper<Args, I>...>::value))...);
    
            ClazzObject<ClassType>::set_propertie(self, ClazzObject<ClassType>::Propertie::Value);
            ClazzObject<ClassType>::set_hash(self);
            
            invoker(ObjectWrapper<Args, I>::destructor(nativeArgsBuffer +
                ObjectOffset<ToNative, ObjectWrapper<Args, I>,
                    ObjectWrapper<Args, I>...>::value)...);
            return 0;
        }

        static int wrapper(PyObject *self, PyObject *args, PyObject *kwargs)
        {
            return wrapper_impl(self, args, std::make_index_sequence<sizeof...(Args)>{});
        }

    private:
        Constructor(){
            static_assert(typeid(enable_if_t<std::is_constructible<ClassType, Args...>::type::value, bool>) == typeid(bool), "Type dosn't supports the associated constructor prototype.");
        };
    };

}
