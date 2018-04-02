#include "CPythonClassTestModule.h"
#include "CPythonModule.h"
#include "CPythonClass.h"

using namespace pycppconn;

namespace pycppconnTest {

    bool TestSubjectA::m_valid = false;
    bool TestSubjectA::m_instanceDestroyed = false;

    INIT_MODULE(CPythonClassTestModule, "A testing module for CPythonClass static method property") {
        CPythonClass<TestSubjectA> subject(module, "TestClass", "A subject usertype for the CPythonClass static method property");
        subject.AddConstructor<int &>();
        subject.AddMethod("GetValue", "GetValue - Will retrive m_intValue", &TestSubjectA::GetValue);
        subject.AddMethod("SetString", "SetStr - Will modify the internal m_str", &TestSubjectA::SetString);
        subject.AddMethod("GetB", "Return an lvalue reference to TestSubjectB instance", &TestSubjectA::GetB);
        subject.AddStaticMethod("Setter", "Setter - will change the value of m_valid into true", &TestSubjectA::Setter);
        subject.AddStaticMethod("Getter", "Getter - will retrieve m_valid", &TestSubjectA::Getter);
        subject.AddStaticMethod("BMutator", "Mutates received TestSubjectB instance", &TestSubjectA::BMutator);
        subject.AddMember("byValueInt", &TestSubjectA::m_byValueInt, "int by value member support");
    }

}
