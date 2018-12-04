#pragma once

#include <Python.h>
#include <string>

namespace sweetPy {
    
class AsciiString {
public:
    AsciiString();
    AsciiString(const std::string& str);
    AsciiString(PyObject *unicodeStr);
    AsciiString(const AsciiString &obj);
    AsciiString &operator=(const AsciiString &rhs);
    AsciiString(AsciiString &&obj);
    AsciiString &operator=(AsciiString &&rhs);
    
    operator const std::string&() const {return m_str;}
    operator std::string() const {return m_str;}
    PyObject* ToPython() const;

private:
    std::string m_str;
};
}
