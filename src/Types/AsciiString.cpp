#include "AsciiString.h"
#include "Core/Assert.h"

namespace sweetPy{
    AsciiString::AsciiString() {}
    AsciiString::AsciiString(const std::string &str):m_str(str) {}
    AsciiString::AsciiString(PyObject *unicodeStr)
    {
        CPYTHON_VERIFY(Py_TYPE(unicodeStr) == &PyUnicode_Type, "Received object is not a unicode object");
        object_ptr bytesObject(PyUnicode_AsASCIIString(unicodeStr), &Deleter::Owner);
        CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
        m_str = PyBytes_AsString(bytesObject.get());
    }
    AsciiString::AsciiString(const sweetPy::AsciiString &obj): m_str(obj.m_str) {}
    AsciiString& AsciiString::operator=(const sweetPy::AsciiString &rhs) {m_str = rhs.m_str; return *this;}
    AsciiString::AsciiString(sweetPy::AsciiString &&obj):m_str(std::move(obj.m_str)){}
    AsciiString& AsciiString::operator=(sweetPy::AsciiString &&rhs) {m_str = std::move(rhs.m_str); return *this;}
    
    PyObject* AsciiString::ToPython() const
    {
        return PyUnicode_FromString(m_str.c_str());
    }
}
