#pragma once

#include <Python.h>
#include <utility>
#include <sstream>
#include "../Types/Tuple.h"
#include "../Types/ObjectPtr.h"
#include "../Detail/CPythonObject.h"
#include "Traits.h"
#include "Deleter.h"

namespace sweetPy
{

    struct Python
    {
    public:
        static ObjectPtr get_attribute(const char* name, PyObject* startingContext = nullptr)
        {
            std::vector<std::string> tokens = split(name);
            std::stringstream ss;
            sweetPy::GilLock lock;
            ObjectPtr context(startingContext ? startingContext : PyImport_AddModule("__main__"), &Deleter::Borrow); //verify that borrow is correct
            for(const std::string& token : tokens){
                ObjectPtr attributeName(PyUnicode_FromString(token.c_str()), &Deleter::Owner);
                {
                    ObjectPtr tmpContext(PyObject_GetAttr(context.get(), attributeName.get()), &Deleter::Owner);
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
        static ObjectPtr invoke_function(const char* moduleName, const char* objectName, const char* functionName, Args&&... args)
        {

            ObjectPtr module(PyImport_ImportModule(moduleName), &Deleter::Owner);
            ObjectPtr function = get_attribute(functionName, module.get());

            ObjectPtr object = strcmp(objectName, "") == 0 ? nullptr : get_attribute(objectName, module.get());
            return invoke_function_impl(function, std::move(object), std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static ObjectPtr invoke_function(const ObjectPtr& function, Args&&... args)
        {
            return invoke_function_impl(function, ObjectPtr(), std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
        }

        static ObjectPtr fast_invoke_function(const ObjectPtr& function, const Tuple& arguments)
        {
            ObjectPtr _arguments(arguments.to_python(), &Deleter::Owner);
            ObjectPtr returnValue(PyObject_CallObject(function.get(), _arguments.get()), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(returnValue.get()!=nullptr);
            return returnValue;
        }

        static void register_on_exit(PyMethodDef& descriptor)
        {
            ObjectPtr cFunction(PyCFunction_NewEx(&descriptor, nullptr, nullptr), &Deleter::Owner);
            invoke_function("atexit", "", "register", cFunction.get());
        }

    private:
        static std::vector<std::string> split(const std::string& str)
        {
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
        static ObjectPtr invoke_function_impl(const ObjectPtr& function, ObjectPtr&& object, std::index_sequence<I...> index, Args&&... args)
        {
            ObjectPtr tuple(nullptr, &Deleter::Owner);
            if(object.get() == nullptr)
            {
                tuple.reset(PyTuple_New(sizeof...(args)));
                invoker(PyTuple_SetItem(tuple.get(), I, Object<Args>::to_python(args))...);
            }
            else
            {
                tuple.reset(PyTuple_New(sizeof...(args) + 1));
                PyTuple_SetItem(tuple.get(), 0, object.release());
                invoker(PyTuple_SetItem(tuple.get(), I + 1, Object<Args>::to_python(args))...);
            }
            ObjectPtr returnValue(PyObject_CallObject(function.get(), tuple.get()), &Deleter::Owner);
            CPYTHON_VERIFY_EXC(returnValue.get() !=nullptr);
            return std::move(returnValue);
        }
    };
}
