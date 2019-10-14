#include "Types/Container.h"
#include "Types/Tuple.h"
#include "Types/List.h"
#include "Types/AsciiString.h"

namespace sweetPy{
    
    void _Container::add_element(const ObjectPtr &element)
    {
        if(element->ob_type == &PyLong_Type)
            m_elements.emplace_back(new core::TypedParam<int>(Object<int>::from_python(element.get())));
        else if(element->ob_type == &PyUnicode_Type)
        {
            auto str = Object<AsciiString>::from_python(element.get());
            const char* ptr = str.get_str().c_str();
            m_elements.emplace_back(new core::TypedParam<char*>(ptr));
        }
        else if(element->ob_type == &PyBytes_Type)
            m_elements.emplace_back(new core::TypedParam<std::string>(Object<std::string>::from_python(element.get())));
        else if(element->ob_type == &PyFloat_Type)
            m_elements.emplace_back(new core::TypedParam<double>(Object<double>::from_python(element.get())));
        else if(element->ob_type == &PyBool_Type)
            m_elements.emplace_back(new core::TypedParam<bool>(Object<bool>::from_python(element.get())));
        else if(element->ob_type == &PyTuple_Type)
            m_elements.emplace_back(new core::TypedParam<Tuple>(Object<Tuple>::from_python(element.get())));
        else if(element->ob_type == &PyList_Type)
            m_elements.emplace_back(new core::TypedParam<List>(Object<List>::from_python(element.get())));
        else if(element.get() == Py_None)
            m_elements.emplace_back(new core::TypedParam<void*>(nullptr));
        else
            throw core::Exception(__CORE_SOURCE, "Non supported python object was provided");
    }
}

