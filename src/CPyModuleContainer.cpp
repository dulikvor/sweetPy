#include "CPyModuleContainer.h"
#include <Python.h>
#include "core/Source.h"
#include "Exception.h"

namespace pycppconn{

    CPyModuleContainer& CPyModuleContainer::Instance() {
        static CPyModuleContainer instance;
        return instance;
    }

    void CPyModuleContainer::AddModule(const std::string &key, const std::shared_ptr<CPythonModule> &module) {
        if( m_modules.find(key) != m_modules.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key already exists - %s", key.c_str());
        m_modules.insert(std::make_pair(key, module));
    }

    CPythonModule& CPyModuleContainer::GetModule(const std::string &key) {
        if( m_modules.find(key) == m_modules.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key related entry dosn't exists - %s", key.c_str());
        return *m_modules[key];
    }

    void CPyModuleContainer::AddType(size_t key, PyTypeObject *const type){
        if( m_types.find(key) != m_types.end())
            return; //types may return due repeating CPythonObjectTypes.
        m_types.insert(std::make_pair(key, type));
    }


    void CPyModuleContainer::AddMethod(int key, const std::shared_ptr<ICPythonFunction>& method){
        if( m_methods.find(key) != m_methods.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key already exists - %d", key);
        m_methods.insert(std::make_pair(key, method));
    }

    void CPyModuleContainer::AddStaticMethod(int key, const std::shared_ptr<ICPythonFunction>& staticMethod){
        if( m_staticMethods.find(key) != m_staticMethods.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key already exists - %d", key);
        m_staticMethods.insert(std::make_pair(key, staticMethod));
    }

    ICPythonFunction& CPyModuleContainer::GetMethod(int key){
        if( m_methods.find(key) == m_methods.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key related entry dosn't exists - %d", key);
        return *m_methods[key];
    }

    ICPythonFunction& CPyModuleContainer::GetStaticMethod(int key){
        if( m_staticMethods.find(key) == m_staticMethods.end())
            throw CPythonException(PyExc_KeyError, __CORE_SOURCE, "Key related entry dosn't exists - %d", key);
        return *m_staticMethods[key];
    }

    bool CPyModuleContainer::Exists(size_t key){
        if( m_types.find(key) == m_types.end())
            return false;
        return true;
    }

    PyTypeObject* const CPyModuleContainer::GetType(size_t key){
        if( m_types.find(key) == m_types.end())
            return nullptr;
        return m_types[key];
    }
}
