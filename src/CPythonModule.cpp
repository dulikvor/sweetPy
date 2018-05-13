#include "CPythonModule.h"
#include "Common.h"
#include "Lock.h"
#include "CPythonClass.h"

using namespace std;

namespace pycppconn{

    CPythonModule::CPythonModule(const string &name, const string &doc): m_module(nullptr, &Deleter::Borrow),
        m_name(name), m_doc(doc){
        {
            GilLock lock;
            m_module.reset(Py_InitModule3(m_name.c_str(), NULL, m_doc.c_str()));
        }
        CPYTHON_VERIFY(m_module.get() != nullptr, "Module registration failed");
    }

    void CPythonModule::AddType(std::unique_ptr<TypeState>&& type) {
        GilLock lock;
        CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), type->Name.c_str(), (PyObject*)type->PyType.operator->()) == 0, "Type registration with module failed");
        m_types.emplace_back(std::move(type));
    }

    PyObject* CPythonModule::GetModule() const
    {
        return m_module.get();
    }
}
