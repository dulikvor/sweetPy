#include "Types/List.h"
#include "Types/Tuple.h"

namespace sweetPy{
    PyObject* List::to_python() const
    {
        ObjectPtr list(PyList_New(m_elements.size()), &Deleter::Owner);
        for(int idx = 0; idx < m_elements.size(); idx++)
        {
            PyObject* object = nullptr;
            if(m_elements[idx]->IsInt())
                object = Object<int>::to_python(static_cast<core::TypedParam<int>&>(*m_elements[idx]).Get<int>());
            else if(m_elements[idx]->IsString())
                object = Object<std::string>::to_python(static_cast<core::TypedParam<std::string>&>(*m_elements[idx]).Get<std::string>());
            else if(m_elements[idx]->IsCtypeS())
                object = Object<const char*>::to_python(static_cast<core::TypedParam<char*>&>(*m_elements[idx]).Get<char*>());
            else if(m_elements[idx]->IsDouble())
                object = Object<double>::to_python(static_cast<core::TypedParam<double>&>(*m_elements[idx]).Get<double>());
            else if(m_elements[idx]->IsBool())
                object = Object<bool>::to_python(static_cast<core::TypedParam<bool>&>(*m_elements[idx]).Get<bool>());
            else if(m_elements[idx]->GetTypeId() == core::TypeIdHelper<Tuple>::GenerateTypeId())
                object = Object<Tuple>::to_python(static_cast<core::TypedParam<Tuple>&>(*m_elements[idx]).Get<Tuple>());
            else if(m_elements[idx]->GetTypeId() == core::TypeIdHelper<List>::GenerateTypeId())
                object = Object<List>::to_python(static_cast<core::TypedParam<List>&>(*m_elements[idx]).Get<List>());
            else
            {
                auto it = m_converters.find(idx);
                if(it != m_converters.end())
                {
                    object = it->second(m_elements[idx]->GetBuffer());
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
    