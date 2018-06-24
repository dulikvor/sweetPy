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
            char* name = new char[m_name.length() + 1];
            std::copy_n(m_name.c_str(), m_name.length(), name);
            name[m_name.length()] = '\0';

            char* doc = new char[m_doc.length() + 1];
            std::copy_n(m_doc.c_str(), m_doc.length(), doc);
            doc[m_doc.length()] = '\0';

            return std::unique_ptr<PyMemberDef>(new PyMemberDef{
                    name,
                    T_INT,
                    m_offset,
                    READONLY,
                    doc
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

