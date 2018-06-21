#pragma once

#include <string>
#include <memory>
#include <Python.h>

namespace pycppconn {

    //vptr is not allowed
    struct CPythonType : public PyTypeObject
    {
    public:
        CPythonType(const std::string& name, const std::string doc)
                :PyTypeObject{}, m_name(name), m_doc(doc){}

        const std::string& GetName() const {return m_name;}
        const std::string& GetDoc() const {return m_doc;}

    protected:
        std::string m_name;
        std::string m_doc;
    };
}
