#pragma once

#include <Python.h>
#include <type_traits>
#include <initializer_list>
#include <utility>
#include <string>
#include "CPythonObject.h"
#include "Common.h"

namespace sweetPy {

    template<typename ClassType, typename... Args>
    class CPythonConstructor {
    public:
        CPythonConstructor(CPythonConstructor &) = delete;
        CPythonConstructor &operator=(CPythonConstructor &) = delete;
        CPythonConstructor(CPythonConstructor &&obj) = delete;
        CPythonConstructor &operator=(CPythonConstructor &&obj) = delete;

        template<std::size_t... I>
        static int WrapperImpl(PyObject *self, PyObject *args, std::index_sequence<I...>) {
            std::initializer_list<const char *> formatList = {Object<typename base<Args>::Type>::Format...};
            std::string format;
            for (auto &subFormat : formatList)
                format += subFormat;

            char pythonArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::FromPythonType...>::value];
            char nativeArgsBuffer[ObjectsPackSize<typename Object<typename base<Args>::Type>::Type...>::value];
            {
                CPYTHON_VERIFY(PyArg_ParseTuple(args, format.c_str(), (pythonArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
                        ObjectWrapper<typename base<Args>::Type, I>...>::value)...), "Invalid argument was provided");
            }
            new((char*)self + sizeof(PyObject))ClassType(std::forward<Args>(Object<typename base<Args>::Type>::GetTyped(
                    pythonArgsBuffer + ObjectOffset<FromPython, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value,
                    nativeArgsBuffer + ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,ObjectWrapper<typename base<Args>::Type, I>...>::value))...);
            ObjectWrapper<int, 0>::MultiInvoker(ObjectWrapper<typename base<Args>::Type, I>::Destructor(nativeArgsBuffer +
                                                                                                        ObjectOffset<ToNative, ObjectWrapper<typename base<Args>::Type, I>,
                                                                                                                    ObjectWrapper<typename base<Args>::Type, I>...>::value)...);
            return 0;
        }

        static int Wrapper(PyObject *self, PyObject *args, PyObject *kwargs) {
            return WrapperImpl(self, args, std::make_index_sequence<sizeof...(Args)>{});
        }

    private:
        CPythonConstructor(){
            static_assert(typeid(typename std::enable_if<std::is_constructible<ClassType, Args...>::type::value, bool>::type) == typeid(bool), "Type dosn't supports the associated constructor prototype.");
        };
    };

}
