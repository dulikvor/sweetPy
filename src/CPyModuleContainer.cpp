#include "CPyModuleContainer.h"
#include <Python.h>
#include "Exception.h"

namespace pycppconn{

    CPyModuleContainer& CPyModuleContainer::Instance() {
        static CPyModuleContainer instance;
        return instance;
    }

    void CPyModuleContainer::AddModule(const std::string &key, const std::shared_ptr<CPyModule> &module) {
        if( m_modules.find(key) != m_modules.end())
            throw CPythonException(PyExc_KeyError, SOURCE, "Key already exists - %s", key.c_str());
        m_modules.insert(std::make_pair(key, module));
    }

    CPyModule& CPyModuleContainer::GetModule(const std::string &key) {
        if( m_modules.find(key) == m_modules.end())
            throw CPythonException(PyExc_KeyError, SOURCE, "Key related entry dosn't exists - %s", key.c_str());
        return *m_modules[key];
    }
}
