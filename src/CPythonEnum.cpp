#include "CPythonEnum.h"

namespace pycppconn
{
    CPythonEnum::CPythonEnum(CPythonModule &module, const std::string &name, const std::string &doc)
    : m_module(module), m_name(name), m_doc(doc){}

    CPythonEnum::~CPythonEnum() {
        m_metaClass.reset(new CPythonMetaClass(m_module, std::string(m_name) + "_MetaClass",
                                               std::string(m_doc) + "_MetaClass", sizeof(int) * m_enumValues.size()));
        for(auto& enumValue :  m_enumValues)
            m_metaClass->AddEnumValue(std::move(enumValue));
        m_metaClass->InitType();
        PyTypeObject* typeInstance = (PyTypeObject*)m_metaClass->ToPython().tp_alloc(&m_metaClass->ToPython(), 0);
        CPYTHON_VERIFY(PyModule_AddObject(m_module.GetModule(), typeInstance->tp_name, (PyObject*)typeInstance) == 0, "Type registration with module failed");
    }
    void CPythonEnum::AddEnumValue(const std::string &name, int value, const std::string &doc) {
        static int enumValuesCount = 0;
        m_enumValues.emplace_back(new CPythonEnumValue(m_name, sizeof(int) * enumValuesCount, m_doc));
    }
}
