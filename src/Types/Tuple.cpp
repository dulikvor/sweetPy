#include "Tuple.h"
#include "CPythonObject.h"

namespace sweetPy{
    
    Tuple::Tuple(PyObject *tuple)
    {
        size_t size = PyTuple_Size(tuple);
        for(int idx = 0; idx < size; idx++)
            AddElement(object_ptr(PyTuple_GetItem(tuple, idx), &Deleter::Borrow));
    }
    
    Tuple::Tuple(const Tuple &obj)
            :_Container(obj){}
    
    Tuple& Tuple::operator=(const Tuple &rhs)
    {
        static_cast<_Container&>(*this) = static_cast<const _Container&>(rhs);
        return *this;
    }
    
    Tuple::Tuple(Tuple &&obj): _Container(std::move(static_cast<_Container&>(obj))){}
    Tuple& Tuple::operator=(Tuple &&rhs)
    {
        static_cast<_Container&>(*this) = std::move(static_cast<_Container&>(rhs));
        return *this;
    }
    
    bool Tuple::operator==(const sweetPy::Tuple &rhs) const
    {
        return static_cast<const _Container&>(*this) == static_cast<const _Container&>(rhs);
    }
    
    PyObject* Tuple::ToPython() const
    {
        object_ptr tuple(PyTuple_New(m_elements.size()), &Deleter::Owner);
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
            PyTuple_SetItem(tuple.get(), idx, object);
        }
        return tuple.release();
    }
}
