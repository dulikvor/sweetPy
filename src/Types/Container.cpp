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

    ObjectPtr _Container::get_element_objectptr(size_t index) const
    {
        PyObject* object = nullptr;
        if(m_elements[index]->IsInt())
            object = Object<int>::to_python(static_cast<core::TypedParam<int>&>(*m_elements[index]).Get<int>());
        else if(m_elements[index]->IsString())
            object = Object<std::string>::to_python(static_cast<core::TypedParam<std::string>&>(*m_elements[index]).Get<std::string>());
        else if(m_elements[index]->IsCtypeS())
            object = Object<const char*>::to_python(static_cast<core::TypedParam<char*>&>(*m_elements[index]).Get<char*>());
        else if(m_elements[index]->IsDouble())
            object = Object<double>::to_python(static_cast<core::TypedParam<double>&>(*m_elements[index]).Get<double>());
        else if(m_elements[index]->IsBool())
            object = Object<bool>::to_python(static_cast<core::TypedParam<bool>&>(*m_elements[index]).Get<bool>());
        else if(m_elements[index]->GetTypeId() == core::TypeIdHelper<Tuple>::GenerateTypeId())
            object = Object<Tuple>::to_python(static_cast<core::TypedParam<Tuple>&>(*m_elements[index]).Get<Tuple>());
        else if(m_elements[index]->GetTypeId() == core::TypeIdHelper<List>::GenerateTypeId())
            object = Object<List>::to_python(static_cast<core::TypedParam<List>&>(*m_elements[index]).Get<List>());
        else
        {
            auto it = m_converters.find(index);
            if(it != m_converters.end())
            {
                object = it->second(m_elements[index]->GetBuffer());
            }
            else
            {
                Py_XINCREF(Py_None);
                object = Py_None;
            }
        }

        return ObjectPtr(object, &Deleter::Owner);
    }
}

