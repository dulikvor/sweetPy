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
        m_metaClass->InitializeSubType(m_name, m_doc);
        m_metaClass->AddToModule();
    }
    void CPythonEnum::AddEnumValue(const std::string &name, int value, const std::string &doc) {
        static int enumValuesCount = 0;
        m_enumValues.emplace_back(new CPythonEnumValue(name, (sizeof(int) * enumValuesCount) + sizeof(PyTypeObject), value, doc));
        enumValuesCount++;
    }
}
