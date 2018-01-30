#include "CPythonModule.h"
#include "Common.h"
#include "Lock.h"

using namespace std;

namespace pycppconn{

    CPythonModule::CPythonModule(const string &name, const string &docString): m_module(nullptr, &Deleter::Borrow) {
        {
            GilLock();
            m_module.reset(Py_InitModule3(name.c_str(), NULL, docString.c_str()));
        }
        CPYTHON_VERIFY(m_module.get() != NULL, "Module registration failed");
    }
}
