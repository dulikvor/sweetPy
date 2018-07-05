#include "CPythonClassTestModule.h"
#include "CPythonModule.h"
#include "CPythonClass.h"
#include "CPythonEnum.h"
#include "CPythonGlobalFunction.h"
#include "InitModule.h"

using namespace sweetPy;

namespace sweetPyTest {

    bool TestSubjectA::m_valid = false;
    bool TestSubjectA::m_instanceDestroyed = false;

    INIT_MODULE(CPythonClassTestModule, "A testing module for CPythonClass static method property") {
        CPythonClass<TestSubjectA> subject(module, "TestClass", "A subject usertype for the CPythonClass static method property");
        subject.AddConstructor<int &>();
        subject.AddMethod("GetMe", "GetMe - Will return a reference to my self", &TestSubjectA::GetMe);
        subject.AddMethod("GetValue", "GetValue - Will retrive m_intValue", &TestSubjectA::GetValue);
        subject.AddMethod("SetString", "SetStr - Will modify the internal m_str", &TestSubjectA::SetString);
        subject.AddMethod("GetB", "Return an lvalue reference to TestSubjectB instance", &TestSubjectA::GetB);
        subject.AddMethod("GetBByValue", "Return a rvalue instance of TestSubjectB being copied from TestSubjectA internal instance", &TestSubjectA::GetBByValue);
        subject.AddMethod("GetBByNoCopyConstructible", "Return a rvalue instance of non copy constructable type", &TestSubjectA::GetBNonCopyConstructable);
        subject.AddMethod("IncBByRef", "Will increase b internal ref count", &TestSubjectA::IncBByRef);
        subject.AddMethod("IncB", "Will increase b internal ref count", &TestSubjectA::IncB);
        subject.AddMethod("IncBaseValue", "Will increase base class member value", &TestSubjectA::IncBaseValue);
        subject.AddMethod("GetBaseValue", "Will get base class member value", &TestSubjectA::GetBaseValue);
        subject.AddStaticMethod("GetUniqueMe", "GetUniqueMe - Return an rvalue reference of non supported type", &TestSubjectA::GetUniqueMe);
        subject.AddStaticMethod("Setter", "Setter - will change the value of m_valid into true", &TestSubjectA::Setter);
        subject.AddStaticMethod("Getter", "Getter - will retrieve m_valid", &TestSubjectA::Getter);
        subject.AddStaticMethod("BMutator", "Mutates received TestSubjectB instance", &TestSubjectA::BMutator);
        subject.AddMember("byValueInt", &TestSubjectA::m_byValueInt, "int by value member support");
        subject.AddMember("ctypeStr", &TestSubjectA::m_ctypeStr, "c-type string member support");
        subject.AddMethod("SetPython", "Will set the python enum value", &TestSubjectA::SetPython);
        subject.AddMethod("GetStr", "Will return a reference to m_str", &TestSubjectA::GetStr);
        subject.AddMethod("SetXpireValue", "Will set m_str from an xpire value", &TestSubjectA::SetXpireValue);

        CPythonClass<TestSubjectB> subjectB(module, "TestClassB", "TestClassB");
        subjectB.AddMethod("IncValue", "Will increase b internal ref count", &TestSubjectB::IncValue);
        subjectB.AddMember("value", &TestSubjectB::m_value, "value");
        subjectB.AddMember("str", &TestSubjectB::m_str, "str");
        subjectB.AddMethod("Foo_1", "Foo function", static_cast<int(TestSubjectB::*)(int)>(&TestSubjectB::Foo));
        subjectB.AddMethod("Foo_2", "Foo function", static_cast<int(TestSubjectB::*)(int, int)>(&TestSubjectB::Foo));

        CPythonEnum enumSubject(module, "Enum_Python", "What we think about python in general");
        enumSubject.AddEnumValue("Good", (int)Python::Good, "We are pretty sure, python is great");
        enumSubject.AddEnumValue("Bad", (int)Python::Bad, "Lets be honest, cpp is the best :)");

        CPythonClass<TestSubjectC> subjectC(module, "TestClassC", "A non copyable/moveable version for a class");
        subjectC.AddMethod("inc", "will increase i", &TestSubjectC::Inc);
        subjectC.AddStaticMethod("instance", "a reference method", &TestSubjectC::Instance);

        CPythonGlobalFunction function(module, "globalFunction", "global function", &globalFunction);
    }

}
