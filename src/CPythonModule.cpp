#include "CPythonModule.h"
#include "CPythonClass.h"
#include "CPythonType.h"

using namespace std;

namespace sweetPy{

    CPythonModule::CPythonModule(const string &name, const string &doc): m_module(nullptr, &Deleter::Borrow),
        m_name(name), m_doc(doc){
        {
            m_module.reset(Py_InitModule3(m_name.c_str(), NULL, m_doc.c_str()));
        }
        CPYTHON_VERIFY(m_module.get() != nullptr, "Module registration failed");
    }

    void CPythonModule::AddType(std::unique_ptr<CPythonType>&& type) {
        CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), type->GetName().c_str(), (PyObject*)type.get()) == 0, "Type registration with module failed");
        m_types.emplace_back(std::move(type));
    }

    PyObject* CPythonModule::GetModule() const
    {
        return m_module.get();
    }
}
