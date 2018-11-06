#pragma once

#include <Python.h>
#include <utility>
#include <sstream>
#include "CPythonObject.h"
#include "Traits.h"
#include "Deleter.h"

namespace sweetPy
{

    struct Python
    {
    public:
        static object_ptr GetAttribute(const char* name, PyObject* startingContext = nullptr)
        {
            std::vector<std::string> tokens = Split(name);
            std::stringstream ss;
            sweetPy::GilLock lock;
            object_ptr context(startingContext ? startingContext : PyImport_AddModule("__main__"), &Deleter::Borrow); //verify that borrow is correct
            for(const std::string& token : tokens){
                object_ptr attributeName(PyUnicode_FromString(token.c_str()), &Deleter::Owner);
                {
                    object_ptr tmpContext(PyObject_GetAttr(context.get(), attributeName.get()), &Deleter::Owner);
                    context.swap(tmpContext);
                }
                if(context.get() == nullptr)
                {
                    ss<<"Was unable to retrieve attribute - "<<token;
                    CPYTHON_VERIFY(false, ss.str().c_str());
                }
            }
            return context;
        }

        template<typename... Args>
        static object_ptr InvokeFunction(const char* moduleName, const char* objectName, const char* functionName, Args&&... args)
        {

            object_ptr module(PyImport_ImportModule(moduleName), &Deleter::Owner);
            object_ptr function = GetAttribute(functionName, module.get());

            object_ptr object = strcmp(objectName, "") == 0 ? nullptr : GetAttribute(objectName, module.get());
            return invoke_function_impl(std::move(function), std::move(object), std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static object_ptr InvokeFunction(object_ptr&& function, Args&&... args)
        {
            return invoke_function_impl(std::move(function), object_ptr(), std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
        }

        static void RegisterOnExit(PyMethodDef& descriptor)
        {
            object_ptr cFunction(PyCFunction_NewEx(&descriptor, nullptr, nullptr), &Deleter::Owner);
            InvokeFunction("atexit", "", "register", cFunction.get());
        }

    private:
        static std::vector<std::string> Split(const std::string& str){
            std::vector<std::string> result;
            std::size_t start = 0, end = 0;
            while((end = str.find('.', start)) != std::string::npos){
                result.emplace_back(str.substr(start, end - start));
                start = end + 1;
            }
            result.emplace_back(str.substr(start));
            return result;
        }

        template<std::size_t... I, typename... Args>
        static object_ptr invoke_function_impl(object_ptr&& function, object_ptr&& object, std::index_sequence<I...> index, Args&&... args)
        {
            object_ptr tuple(nullptr, &Deleter::Owner);
            if(object.get() == nullptr)
            {
                tuple.reset(PyTuple_New(sizeof...(args)));
                ObjectWrapper<int, 0>::MultiInvoker(PyTuple_SetItem(tuple.get(), I, Object<Args>::ToPython(args))...);
            }
            else
            {
                tuple.reset(PyTuple_New(sizeof...(args) + 1));
                PyTuple_SetItem(tuple.get(), 0, object.release());
                ObjectWrapper<int, 0>::MultiInvoker(PyTuple_SetItem(tuple.get(), I + 1, Object<Args>::ToPython(args))...);
            }
            object_ptr returnValue(PyObject_CallObject(function.get(), tuple.get()), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(returnValue.get() !=nullptr);
            return returnValue;
        }
    };
}
