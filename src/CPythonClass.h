#pragma once

#include <type_traits>
#include <vector>
#include <memory>
#include <Python.h>
#include "CPythonFunction.h"

namespace pycppconn{

    template<typename T, typename Type = typename std::remove_const<typename std::remove_pointer<
            typename std::remove_reference<T>::type>::type>::type>
    class CPythonClass {
    public:
        template<typename X, typename std::enable_if<std::__and_<std::is_pointer<X>,
                std::is_function<typename std::remove_pointer<X>::type>>::value, bool>::type = true>
        void AddMethod(const std::string& name, const std::string& doc, X&& memberFunction){
            m_cPythonFunctions.emplace_back(new CPythonFunction<X>(name, doc, memberFunction));
        }
        std::unique_ptr<PyTypeObject> ToPython() const;
    private:
        std::vector<std::unique_ptr<ICPythonFunction>> m_cPythonFunctions;
    };
}
