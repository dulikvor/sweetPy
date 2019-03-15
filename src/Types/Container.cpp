#include "Container.h"
#include "CPythonObject.h"

namespace sweetPy{
    
    _Container::Converter::Converter(void const * value, const ConveterFunc &converter): m_value(value), m_converter(converter) {}
    
    _Container::Converter::operator PyObject *() const
    {
        return m_converter(m_value);
    }
    
    _Container::_Container(const _Container &obj)
    {
        for(auto& element : obj.m_elements)
        {
            std::unique_ptr<core::Param> param;
            if(element->IsInt())
            {
                core::TypedParam<int>& typedElement =  static_cast<core::TypedParam<int>&>(*element);
                param.reset(new core::TypedParam<int>(typedElement));
            }
            else if(element->IsBool())
            {
                core::TypedParam<bool>& typedElement =  static_cast<core::TypedParam<bool>&>(*element);
                param.reset(new core::TypedParam<bool>(typedElement));
            }
            else if(element->IsDouble())
            {
                core::TypedParam<double>& typedElement =  static_cast<core::TypedParam<double>&>(*element);
                param.reset(new core::TypedParam<double>(typedElement));
            }
            else if(element->IsString())
            {
                core::TypedParam<std::string>& typedElement =  static_cast<core::TypedParam<std::string>&>(*element);
                param.reset(new core::TypedParam<std::string>(typedElement));
            }
            else if(element->IsPointer())
            {
                core::TypedParam<void *> &typedElement = static_cast<core::TypedParam<void *> &>(*element);
                param.reset(new core::TypedParam<void *>(typedElement));
            }
            m_elements.emplace_back(std::move(param));
        }
    }
    
    _Container& _Container::operator=(const _Container &rhs)
    {
        for(auto& element : rhs.m_elements)
        {
            std::unique_ptr<core::Param> param;
            if(element->IsInt())
            {
                core::TypedParam<int>& typedElement =  static_cast<core::TypedParam<int>&>(*element);
                param.reset(new core::TypedParam<int>(typedElement));
            }
            else if(element->IsBool())
            {
                core::TypedParam<bool>& typedElement =  static_cast<core::TypedParam<bool>&>(*element);
                param.reset(new core::TypedParam<bool>(typedElement));
            }
            else if(element->IsDouble())
            {
                core::TypedParam<double>& typedElement =  static_cast<core::TypedParam<double>&>(*element);
                param.reset(new core::TypedParam<double>(typedElement));
            }
            else if(element->IsString())
            {
                core::TypedParam<std::string>& typedElement =  static_cast<core::TypedParam<std::string>&>(*element);
                param.reset(new core::TypedParam<std::string>(typedElement));
            }
            else if(element->IsPointer())
            {
                core::TypedParam<void *> &typedElement = static_cast<core::TypedParam<void *> &>(*element);
                param.reset(new core::TypedParam<void *>(typedElement));
            }
            m_elements.emplace_back(std::move(param));
        }
        return *this;
    }
    
    _Container::_Container(_Container &&obj) NOEXCEPT(true) : m_elements(std::move(obj.m_elements)){}
    _Container& _Container::operator=(_Container &&rhs) NOEXCEPT(true){m_elements = std::move(rhs.m_elements); return *this;}
    
    bool _Container::operator==(const sweetPy::_Container &rhs) const
    {
        bool rangeEqual = std::equal(begin(), end(), rhs.begin(), [](_Container::const_iterator::reference lhs, _Container::const_iterator::reference rhs){
            if(lhs.IsInt())
            {
                if(rhs.IsInt() == false)
                    return false;
                const core::TypedParam<int> &typedElement_lhs = static_cast<const core::TypedParam<int> &>(lhs);
                const core::TypedParam<int> &typedElement_rhs = static_cast<const core::TypedParam<int> &>(rhs);
                if(typedElement_lhs.Get<int>() != typedElement_rhs.Get<int>())
                    return false;
            }
            else if(lhs.IsDouble())
            {
                if(rhs.IsDouble() == false)
                    return false;
                const core::TypedParam<double> &typedElement_lhs = static_cast<const core::TypedParam<double> &>(lhs);
                const core::TypedParam<double> &typedElement_rhs = static_cast<const core::TypedParam<double> &>(rhs);
                if(typedElement_lhs.Get<double>() != typedElement_rhs.Get<double>())
                    return false;
            }
            else if(lhs.IsBool())
            {
                if(rhs.IsBool() == false)
                    return false;
                const core::TypedParam<bool> &typedElement_lhs = static_cast<const core::TypedParam<bool> &>(lhs);
                const core::TypedParam<bool> &typedElement_rhs = static_cast<const core::TypedParam<bool> &>(rhs);
                if(typedElement_lhs.Get<bool>() != typedElement_rhs.Get<bool>())
                    return false;
            }
            else if(lhs.IsString())
            {
                if(rhs.IsString() == false)
                    return false;
                const core::TypedParam<std::string> &typedElement_lhs = static_cast<const core::TypedParam<std::string> &>(lhs);
                const core::TypedParam<std::string> &typedElement_rhs = static_cast<const core::TypedParam<std::string> &>(rhs);
                if(typedElement_lhs.Get<std::string>() != typedElement_rhs.Get<std::string>())
                    return false;
            }
            else if(lhs.IsPointer())
            {
                if(rhs.IsPointer() == false)
                    return false;
                const core::TypedParam<void*> &typedElement_lhs = static_cast<const core::TypedParam<void*> &>(lhs);
                const core::TypedParam<void*> &typedElement_rhs = static_cast<const core::TypedParam<void*> &>(rhs);
                if(typedElement_lhs.Get<void*>() != typedElement_rhs.Get<void*>())
                    return false;
            }
            return true;
        });
        
        return rangeEqual && m_converters == rhs.m_converters;
    }
    
    void _Container::AddElement(const object_ptr &element)
    {
        if(element->ob_type == &PyLong_Type)
            m_elements.emplace_back(new core::TypedParam<int>(Object<int>::FromPython(element.get())));
        else if(element->ob_type == &PyUnicode_Type)
            m_elements.emplace_back(new core::TypedParam<std::string>(Object<std::string>::FromPython(element.get())));
        else if(element->ob_type == &PyBytes_Type)
            m_elements.emplace_back(new core::TypedParam<std::string>(Object<std::string>::FromPython(element.get())));
        else if(element->ob_type == &PyFloat_Type)
            m_elements.emplace_back(new core::TypedParam<double>(Object<double>::FromPython(element.get())));
        else if(element->ob_type == &PyBool_Type)
            m_elements.emplace_back(new core::TypedParam<bool>(Object<bool>::FromPython(element.get())));
        else if(element.get() == Py_None)
            m_elements.emplace_back(new core::TypedParam<void*>(nullptr));
        else
            throw core::Exception(__CORE_SOURCE, "Non supported python object was provided");
    }
    
    void _Container::Clear()
    {
        m_elements.clear();
    }
}

