#pragma once

#include <string>
#include <memory>
#include <Python.h>
#include "CPythonModule.h"
#include "CPythonMetaClass.h"
#include "CPythonEnumValue.h"

namespace pycppconn {

    class CPythonEnum {
    public:
        CPythonEnum(CPythonModule &module, const std::string &name, const std::string &doc);
        ~CPythonEnum();
        template<typename T>
        void AddEnumValue(const std::string &name, T&& value, const std::string &doc)
        {
            static int enumValuesCount = 0;
            int offset = (sizeof(int) * enumValuesCount++) + sizeof(PyTypeObject);
            m_enumValues.emplace_back(new CPythonEnumValue(name, offset, value, doc));
        }

    private:
        std::unique_ptr<CPythonMetaClass> m_metaClass;
        std::vector<std::unique_ptr<CPythonEnumValue>> m_enumValues;
        std::string m_name;
        std::string m_doc;
        CPythonModule &m_module;
    };
}
