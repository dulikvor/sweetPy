#pragma once

#include <Python.h>
#include <string>
#include <vector>
#include "Types/ObjectPtr.h"
#include "Module.h"

namespace sweetPy {

    class Enum {
    public:
        Enum(Module &module, const std::string &name)
            : m_name(name), m_module(module){}
        ~Enum()
        {
            ObjectPtr dict(PyDict_New(), &Deleter::Owner);
            for(auto& enumValue : m_enumValues)
            {
                ObjectPtr key(PyUnicode_FromString(enumValue.first.c_str()), &Deleter::Owner); //PyUnicode_FromString copies the decoded bytes
                ObjectPtr value(PyLong_FromLong(enumValue.second), &Deleter::Owner);
                CPYTHON_VERIFY(PyDict_SetItem(dict.get(), key.get(), value.get()) == 0, "Was unable to insert key-value into the dictionary");
            }
            m_module.add_enum(m_name, std::move(dict));
        }
        void add_value(const std::string &name, int value)
        {
            m_enumValues.emplace_back(name, value);
        }

    private:
        typedef std::pair<std::string, int> EnumPair;
        std::vector<EnumPair> m_enumValues;
        std::string m_name;
        Module &m_module;
    };
}
