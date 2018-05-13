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
        void AddEnumValue(const std::string &name, int value, const std::string &doc);

    private:
        std::unique_ptr<CPythonMetaClass> m_metaClass;
        std::vector<std::unique_ptr<CPythonEnumValue>> m_enumValues;
        std::string m_name;
        std::string m_doc;
        CPythonModule &m_module;
    };
}
