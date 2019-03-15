#include "List.h"
#include "CPythonObject.h"

namespace sweetPy{
    
    List::List(PyObject *list)
    {
        size_t size = PyList_Size(list);
        for(int idx = 0; idx < size; idx++)
            AddElement(object_ptr(PyList_GetItem(list, idx), &Deleter::Borrow));
    }
    
    List::List(const List &obj)
            :_Container(obj){}
    
    List& List::operator=(const List &rhs)
    {
        static_cast<_Container&>(*this) = static_cast<const _Container&>(rhs);
        return *this;
    }
    
    List::List(List &&obj) NOEXCEPT(true) : _Container(std::move(static_cast<_Container&>(obj))){}
    List& List::operator=(List &&rhs) NOEXCEPT(true)
    {
        static_cast<_Container&>(*this) = std::move(static_cast<_Container&>(rhs));
        return *this;
    }
    
    bool List::operator==(const sweetPy::List &rhs) const
    {
        return static_cast<const _Container&>(*this) == static_cast<const _Container&>(rhs);
    }
    
    PyObject* List::ToPython() const
    {
        object_ptr list(PyList_New(m_elements.size()), &Deleter::Owner);
        for(int idx = 0; idx < m_elements.size(); idx++)
        {
            PyObject* object = nullptr;
            if(m_elements[idx]->IsInt())
                object = Object<int>::ToPython(static_cast<core::TypedParam<int>&>(*m_elements[idx]).Get<int>());
            else if(m_elements[idx]->IsString())
                object = Object<std::string>::ToPython(static_cast<core::TypedParam<std::string>&>(*m_elements[idx]).Get<std::string>());
            else if(m_elements[idx]->IsDouble())
                object = Object<double>::ToPython(static_cast<core::TypedParam<double>&>(*m_elements[idx]).Get<double>());
            else if(m_elements[idx]->IsBool())
                object = Object<bool>::ToPython(static_cast<core::TypedParam<bool>&>(*m_elements[idx]).Get<bool>());
            else
            {
                auto it = m_converters.find(idx);
                if(it != m_converters.end())
                {
                    object = it->second;
                }
                else
                {
                    Py_XINCREF(Py_None);
                    object = Py_None;
                }
            }
            PyList_SetItem(list.get(), idx, object);
        }
        return list.release();
    }
}

