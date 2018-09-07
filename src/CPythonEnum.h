#pragma once

#include <Python.h>
#include <string>
#include <vector>
#include "CPythonModule.h"

namespace sweetPy {

    class CPythonEnum {
    public:
        CPythonEnum(CPythonModule &module, const std::string &name);
        ~CPythonEnum();
        void AddEnumValue(const std::string &name, int value);

    private:
        std::vector<std::pair<std::string, int>> m_enumValues;
        std::string m_name;
        CPythonModule &m_module;
    };
}
