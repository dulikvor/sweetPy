#include "CPythonEnum.h"
#include "Core/Deleter.h"
#include "Core/Assert.h"

namespace sweetPy
{
    CPythonEnum::CPythonEnum(CPythonModule &module, const std::string &name)
    : m_module(module), m_name(name){}

    CPythonEnum::~CPythonEnum() {
        typedef std::unique_ptr<PyObject, Deleter::Func> object_ptr;
        object_ptr dict(PyDict_New(), &Deleter::Owner);
        for(auto& enumValue : m_enumValues)
        {
            object_ptr key(PyUnicode_FromString(enumValue.first.c_str()), &Deleter::Owner); //PyUnicode_FromString copies the decoded bytes
            object_ptr value(PyLong_FromLong(enumValue.second), &Deleter::Owner);
            CPYTHON_VERIFY(PyDict_SetItem(dict.get(), key.get(), value.get()) == 0, "Was unable to insert key-value into the dictionary");
        }
        m_module.AddEnum(m_name, std::move(dict));
    }

    void CPythonEnum::AddEnumValue(const std::string &name, int value)
    {
       m_enumValues.emplace_back(name, value);
    }
}
