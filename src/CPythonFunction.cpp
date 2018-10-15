#include "CPythonFunction.h"

namespace sweetPy{


    CPythonFunction::CPythonFunction(const std::string& name, const std::string& doc)
            :m_name(name), m_doc(doc){}

    CPythonFunction::~CPythonFunction(){}

    CPythonFunction::CPythonFunction(CPythonFunction &&obj)
        :m_name(std::move(obj.m_name)), m_doc(std::move(obj.m_doc)) {}

    CPythonFunction& CPythonFunction::operator=(CPythonFunction &&obj)
    {
        std::swap(m_name, obj.m_name);
        std::swap(m_doc, obj.m_doc);
        return *this;
    }
}
