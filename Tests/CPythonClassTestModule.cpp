#include "CPythonClassTestModule.h"
#include "CPythonModule.h"
#include "CPythonClass.h"
#include "CPythonEnum.h"
#include "CPythonGlobalFunction.h"
#include "CPythonGlobalVariable.h"
#include "InitModule.h"

using namespace sweetPy;

namespace sweetPyTest {

    bool TestSubjectA::m_valid = false;
    bool TestSubjectA::m_instanceDestroyed = false;

    INIT_MODULE(CPythonClassTestModule, "A testing module for CPythonClass static method property") {
        CPythonClass<TestSubjectA> subject(module, "TestClass", "A subject usertype for the CPythonClass static method property");
        subject.AddConstructor<int>();
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
        subject.AddMethod("FromStrVectorToIntVector", "Will transform str vector to int vector", &TestSubjectA::FromStrVectorToIntVector);

        CPythonClass<TestSubjectB> subjectB(module, "TestClassB", "TestClassB");
        subjectB.AddMethod("IncValue", "Will increase b internal ref count", &TestSubjectB::IncValue);
        subjectB.AddMember("value", &TestSubjectB::m_value, "value");
        subjectB.AddMember("str", &TestSubjectB::m_str, "str");
        subjectB.AddMethod("Foo_1", "Foo function", static_cast<int(TestSubjectB::*)(int)>(&TestSubjectB::Foo));
        subjectB.AddMethod("Foo_2", "Foo function", static_cast<int(TestSubjectB::*)(int, int)>(&TestSubjectB::Foo));

        CPythonEnum enumSubject(module, "Enum_Python");
        enumSubject.AddEnumValue("Good", (int)Python::Good);
        enumSubject.AddEnumValue("Bad", (int)Python::Bad);

        CPythonClass<TestSubjectC> subjectC(module, "TestClassC", "A non copyable/moveable version for a class");
        subjectC.AddMethod("inc", "will increase i", &TestSubjectC::Inc);
        subjectC.AddStaticMethod("instance", "a reference method", &TestSubjectC::Instance);

        CPythonClass<GenerateRefTypes<int>> intRefType(module, "GenerateIntRef", "Will generate instance of int ref type");
        intRefType.AddMethod("create", "Will generate an int ref", static_cast<int&(GenerateRefTypes<int>::*)(const int&)>(&GenerateRefTypes<int>::operator()));

        CPythonClass<GenerateRefTypes<const int>> intConstRefType(module, "GenerateIntConstRef", "Will generate instance of int const ref type");
        intConstRefType.AddMethod("create", "Will generate an int ref", static_cast<const int&(GenerateRefTypes<const int>::*)(const int&)>(&GenerateRefTypes<const int>::operator()));

        CPythonClass<GenerateRefTypes<double>> doubleRefType(module, "GenerateDoubleRef", "Will generate instance of double ref type");
        doubleRefType.AddMethod("create", "Will generate an double ref", static_cast<double&(GenerateRefTypes<double>::*)(const double&)>(&GenerateRefTypes<double>::operator()));

        CPythonClass<GenerateRefTypes<const double>> doubleConstRefType(module, "GenerateDoubleConstRef", "Will generate instance of double const ref type");
        doubleConstRefType.AddMethod("create", "Will generate an double ref", static_cast<const double&(GenerateRefTypes<const double>::*)(const double&)>(&GenerateRefTypes<const double>::operator()));

        CPythonClass<GenerateRefTypes<std::string>> strRefType(module, "GenerateStrRef", "Will generate instance of string ref type");
        strRefType.AddMethod("create", "Will generate an str ref", static_cast<std::string&(GenerateRefTypes<std::string>::*)(const std::string&)>(&GenerateRefTypes<std::string>::operator()));

        CPythonClass<GenerateRefTypes<const std::string>> strConstRefType(module, "GenerateConstStrRef", "Will generate instance of const string ref type");
        strConstRefType.AddMethod("create", "Will generate an const str ref", static_cast<const std::string&(GenerateRefTypes<const std::string>::*)(const std::string&)>(&GenerateRefTypes<const std::string>::operator()));

        CPythonClass<GenerateRefTypes<char*>> ctypeStringRefType(module, "GenerateCTypeStringRef", "Will generate instance of c-type string ref type");
        ctypeStringRefType.AddMethod("create", "Will generate c-type string ref", static_cast<char*&(GenerateRefTypes<char*>::*)(char*)>(&GenerateRefTypes<char*>::operator()));

        CPythonClass<GenerateRefTypes<const sweetPy::DateTime>> dateTimeConstRefType(module, "GenerateDateTimeConstRef", "Will generate instance of datetime const ref type");
        dateTimeConstRefType.AddMethod("create", "Will generate datetime const ref", static_cast<const sweetPy::DateTime&(GenerateRefTypes<const sweetPy::DateTime>::*)(const sweetPy::DateTime&)>(&GenerateRefTypes<const sweetPy::DateTime>::operator()));

        CPythonClass<GenerateRefTypes<const sweetPy::TimeDelta>> timeDeltaConstRefType(module, "GenerateTimeDeltaConstRef", "Will generate instance of timedelta const ref type");
        timeDeltaConstRefType.AddMethod("create", "Will generate timedelta const ref", static_cast<const sweetPy::TimeDelta&(GenerateRefTypes<const sweetPy::TimeDelta>::*)(const sweetPy::TimeDelta&)>(&GenerateRefTypes<const sweetPy::TimeDelta>::operator()));

        CPythonClass<GenerateRefTypes<const sweetPy::Tuple>> tupleConstRefType(module, "GenerateTupleConstRef", "Will generate instance of tuple const ref type");
        tupleConstRefType.AddMethod("create", "Will generate tuple const ref", static_cast<const sweetPy::Tuple&(GenerateRefTypes<const sweetPy::Tuple>::*)(const sweetPy::Tuple&)>(&GenerateRefTypes<const sweetPy::Tuple>::operator()));
        
        CPythonClass<GenerateRefTypes<const sweetPy::AsciiString>> asciiStringConstRefType(module, "GenerateAsciiStringConstRef", "Will generate instance of asciistr const ref type");
        asciiStringConstRefType.AddMethod("create", "Will generate asciistr const ref", static_cast<const sweetPy::AsciiString&(GenerateRefTypes<const sweetPy::AsciiString>::*)(const sweetPy::AsciiString&)>(&GenerateRefTypes<const sweetPy::AsciiString>::operator()));
        
        CPythonClass<GenerateRefTypes<const sweetPy::object_ptr>> objectPtrConstRefType(module, "GenerateObjectPtrConstRef", "Will generate instance of object_ptr const ref type");
        objectPtrConstRefType.AddMethod("create", "Will generate object_ptr const ref", static_cast<const sweetPy::object_ptr&(GenerateRefTypes<const sweetPy::object_ptr>::*)(const sweetPy::object_ptr&)>(&GenerateRefTypes<const sweetPy::object_ptr>::operator()));

        CPythonGlobalFunction(module, "globalFunction", "global function", &globalFunction);
        CPythonGlobalFunction(module, "check_int_conversion", "check integral int type conversions", static_cast<int(*)(int)>(&CheckIntegralIntType));
        CPythonGlobalFunction(module, "check_const_ref_int_conversion", "check integral const ref int type conversions", static_cast<const int&(*)(const int&)>(&CheckIntegralIntType));
        CPythonGlobalFunction(module, "check_ref_int_conversion", "check integral ref int type conversions", static_cast<int&(*)(int&)>(&CheckIntegralIntType));
        CPythonGlobalFunction(module, "check_rvalue_ref_int_conversion", "check integral rvalue ref int type conversions", static_cast<void(*)(int&&)>(&CheckIntegralIntType));
        CPythonGlobalFunction(module, "check_double_conversion", "check integral double type conversions", static_cast<double(*)(double)>(&CheckIntegralDoubleType));
        CPythonGlobalFunction(module, "check_const_ref_double_conversion", "check integral const ref double type conversions", static_cast<const double&(*)(const double&)>(&CheckIntegralDoubleType));
        CPythonGlobalFunction(module, "check_ref_double_conversion", "check integral ref double type conversions", static_cast<double&(*)(double&)>(&CheckIntegralDoubleType));
        CPythonGlobalFunction(module, "check_rvalue_ref_double_conversion", "check integral rvalue ref double type conversions", static_cast<void(*)(double&&)>(&CheckIntegralDoubleType));
        CPythonGlobalFunction(module, "check_str_conversion", "check integral string type conversions", static_cast<std::string(*)(std::string)>(&CheckIntegralStringType));
        CPythonGlobalFunction(module, "check_const_ref_str_conversion", "check integral const ref string type conversions", static_cast<const std::string&(*)(const std::string&)>(&CheckIntegralStringType));
        CPythonGlobalFunction(module, "check_ref_str_conversion", "check integral ref string type conversions", static_cast<std::string&(*)(std::string&)>(&CheckIntegralStringType));
        CPythonGlobalFunction(module, "check_rvalue_ref_str_conversion", "check integral rvalue ref string type conversions", static_cast<void(*)(std::string&&)>(&CheckIntegralStringType));
        CPythonGlobalFunction(module, "check_ref_chararray_conversion", "check integral char array type conversions", static_cast<char(&(*)(char(&)[100]))[100]>(&CheckIntegralCharArrayType));
        CPythonGlobalFunction(module, "check_ctype_string_conversion", "check integral ctype string type conversions", static_cast<void(*)(char*)>(&CheckIntegralCTypeStringType));
        CPythonGlobalFunction(module, "check_const_ctype_string_conversion", "check integral ctype string type conversions", static_cast<void(*)(const char*)>(&CheckIntegralConstCTypeStringType));
        CPythonGlobalFunction(module, "check_pyobject_conversion", "check integral PyObject type conversions", static_cast<PyObject*(*)(PyObject*)>(&CheckIntegralPyObjectType));
        CPythonGlobalFunction(module, "check_datetime_conversion", "check DateTime type conversions", static_cast<sweetPy::DateTime(*)(sweetPy::DateTime)>(&CheckDateTimeType));
        CPythonGlobalFunction(module, "check_const_ref_datetime_conversion", "check const DateTime& type conversions", static_cast<const sweetPy::DateTime&(*)(const sweetPy::DateTime&)>(&CheckConstRefDateTimeType));
        CPythonGlobalFunction(module, "check_timedelta_conversion", "check TimeDelta type conversions", static_cast<sweetPy::TimeDelta(*)(sweetPy::TimeDelta)>(&CheckTimeDeltaType));
        CPythonGlobalFunction(module, "check_const_ref_timedelta_conversion", "check const TimeDelta& type conversions", static_cast<const sweetPy::TimeDelta&(*)(const sweetPy::TimeDelta&)>(&CheckConstRefTimeDeltaType));
        CPythonGlobalFunction(module, "check_tuple_conversion", "check Tuple type conversions", static_cast<sweetPy::Tuple(*)(sweetPy::Tuple)>(&CheckTuleType));
        CPythonGlobalFunction(module, "check_const_ref_tuple_conversion", "check const Tuple& type conversions", static_cast<const sweetPy::Tuple&(*)(const sweetPy::Tuple&)>(&CheckConstRefTupleType));
        CPythonGlobalFunction(module, "generate_native_element_tuple", "Will generate a native tuple which will hold non supported element type", static_cast<sweetPy::Tuple(*)()>(&GenerateNativeElementTuple));
        CPythonGlobalFunction(module, "check_asciistr_conversion", "check AsciiString type conversions", static_cast<sweetPy::AsciiString(*)(sweetPy::AsciiString)>(&CheckAsciiStringType));
        CPythonGlobalFunction(module, "check_const_ref_asciistr_conversion", "check const AsciiString& type conversions", static_cast<const sweetPy::AsciiString&(*)(const sweetPy::AsciiString&)>(&CheckConstRefAsciiStringType));
        CPythonGlobalFunction(module, "check_objectptr_conversion", "check object_ptr type conversions", static_cast<sweetPy::object_ptr(*)(sweetPy::object_ptr)>(&CheckObjectPtrType));
        CPythonGlobalFunction(module, "check_const_ref_objectptr_conversion", "check const object_ptr& type conversions", static_cast<const sweetPy::object_ptr&(*)(const sweetPy::object_ptr&)>(&CheckConstRefObjectPtrType));

        CPythonGlobalVariable(module, "globalVariableStr", "Hello World");
        CPythonGlobalVariable(module, "globalVariableInt", 5);
    }

}
