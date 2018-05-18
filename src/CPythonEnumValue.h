#pragma once

#include <string>
#include <memory>
#include <type_traits>
#include <Python.h>
#include <structmember.h>

namespace pycppconn{

    class CPythonEnumValue
    {
    public:
        CPythonEnumValue(const std::string& name, int offset, int value, const std::string& doc)
                :m_offset(offset), m_name(name), m_doc(doc), m_value(value){}

        std::unique_ptr<PyMemberDef> ToPython() const {
            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                    const_cast<char *>(m_name.c_str()),
                    T_INT,
                    m_offset,
                    READONLY,
                    const_cast<char *>(m_doc.c_str())
            });
        }
        int GetOffset() const { return m_offset; }
        int GetValue() const { return m_value; }

    private:
        std::string m_name;
        std::string m_doc;
        int m_offset;
        int m_value;
    };
}

