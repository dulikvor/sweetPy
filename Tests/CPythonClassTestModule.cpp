#include "CPythonModule.h"
#include "CPythonClass.h"

using namespace pycppconn;

class CPythonClassTestSubject{
public:
    CPythonClassTestSubject(){}
    void static Setter(){
        m_valid = true;
    }

public:
    static bool m_valid;
};

bool CPythonClassTestSubject::m_valid = false;

INIT_MODULE(CPythonClassTestModule, "A testing module for CPythonClass static method property")
{
    CPythonClass<CPythonClassTestSubject> subject(module, "TestClass", "A subject usertype for the CPythonClass static method property");
    subject.AddConstructor<>();
    subject.AddStaticMethod("Setter", "Setter - will change the value of m_valid into true", &CPythonClassTestSubject::Setter);
}
