#include "CPythonEnum.h"
#include "CPythonMetaClass.h"

namespace sweetPy
{
    CPythonEnumType::CPythonEnumType(const std::string& name, const std::string& doc, PyTypeObject* const type)
            :CPythonType(name, doc)
    {
        ob_type = type;
        ob_refcnt = 1;
        ob_size = 0;
        tp_name = m_name.c_str();
        tp_basicsize = sizeof(PyObject);
        tp_flags = Py_TPFLAGS_HAVE_CLASS |
                   Py_TPFLAGS_HAVE_WEAKREFS;
        tp_doc = m_doc.c_str();
    }

    CPythonEnum::CPythonEnum(CPythonModule &module, const std::string &name, const std::string &doc)
    : m_module(module), m_name(name), m_doc(doc){}

    CPythonEnum::~CPythonEnum() {
        m_metaClass.reset(new CPythonMetaClass<true>(m_module, std::string(m_name) + "_MetaClass",
                                               std::string(m_doc) + "_MetaClass", (sizeof(int) * m_enumValues.size())));
        for(auto& enumValue :  m_enumValues)
            m_metaClass->AddEnumValue(std::move(enumValue));
        m_metaClass->InitType();
        m_metaClass->InitializeEnumType(m_name, m_doc);
    }
}
