#include "CPythonClassTestModule.h"

using namespace sweetPy;

namespace sweetPyTest {

    bool TestSubjectA::m_valid = false;
    bool TestSubjectA::m_instanceDestroyed = false;

    INIT_MODULE(CPythonClassTestModule, "A testing module for CPythonClass static method property") {
        
        Clazz<TestSubjectA> subject(module, "TestClass", "A subject usertype for the CPythonClass static method property");
        subject.add_constructor<int>();
        subject.add_method("GetMe", "GetMe - Will return a reference to my self", &TestSubjectA::GetMe);
        subject.add_method("GetValue", "GetValue - Will retrive m_intValue", &TestSubjectA::GetValue);
        subject.add_method("SetString", "SetStr - Will modify the internal m_str", &TestSubjectA::SetString);
        subject.add_method("GetB", "Return an lvalue reference to TestSubjectB instance", &TestSubjectA::GetB);
        subject.add_method("GetBByValue", "Return a rvalue instance of TestSubjectB being copied from TestSubjectA internal instance", &TestSubjectA::GetBByValue);
        subject.add_method("GetBByNoCopyConstructible", "Return a rvalue instance of non copy constructable type", &TestSubjectA::GetBNonCopyConstructable);
        subject.add_method("IncBByRef", "Will increase b internal ref count", &TestSubjectA::IncBByRef);
        subject.add_method("IncB", "Will increase b internal ref count", &TestSubjectA::IncB);
        subject.add_method("IncBaseValue", "Will increase base class member value", &TestSubjectA::IncBaseValue);
        subject.add_method("GetBaseValue", "Will get base class member value", &TestSubjectA::GetBaseValue);
        subject.add_static_method("GetUniqueMe", "GetUniqueMe - Return an rvalue reference of non supported type", &TestSubjectA::GetUniqueMe);
        subject.add_static_method("Setter", "Setter - will change the value of m_valid into true", &TestSubjectA::Setter);
        subject.add_static_method("Getter", "Getter - will retrieve m_valid", &TestSubjectA::Getter);
        subject.add_static_method("BMutator", "Mutates received TestSubjectB instance", &TestSubjectA::BMutator);
        subject.add_member("byValueInt", &TestSubjectA::m_byValueInt, "int by value member support");
        subject.add_member("ctypeStr", &TestSubjectA::m_ctypeStr, "c-type string member support");
        subject.add_method("SetPython", "Will set the python enum value", &TestSubjectA::SetPython);
        subject.add_method("GetStr", "Will return a reference to m_str", &TestSubjectA::GetStr);
        subject.add_method("SetXpireValue", "Will set m_str from an xpire value", &TestSubjectA::SetXpireValue);
        subject.add_method("FromStrVectorToIntVector", "Will transform str vector to int vector", &TestSubjectA::FromStrVectorToIntVector);

        Clazz<TestSubjectB> subjectB(module, "TestClassB", "TestClassB");
        subjectB.add_method("IncValue", "Will increase b internal ref count", &TestSubjectB::IncValue);
        subjectB.add_member("value", &TestSubjectB::m_value, "value");
        subjectB.add_member("str", &TestSubjectB::m_str, "str");
        subjectB.add_method("Foo_1", "Foo function", static_cast<int(TestSubjectB::*)(int)>(&TestSubjectB::Foo));
        subjectB.add_method("Foo_2", "Foo function", static_cast<int(TestSubjectB::*)(int, int)>(&TestSubjectB::Foo));
        
        Enum enumSubject(module, "Enum_Python");
        enumSubject.add_value("Good", (int)Python::Good);
        enumSubject.add_value("Bad", (int)Python::Bad);

        Clazz<TestSubjectC> subjectC(module, "TestClassC", "A non copyable/moveable version for a class");
        subjectC.add_constructor<TestSubjectC&&>();
        subjectC.add_method("inc", "will increase i", &TestSubjectC::Inc);
        subjectC.add_static_method("instance", "a reference method", &TestSubjectC::Instance);

        Clazz<GenerateRefTypes<int>> intRefType(module, "GenerateIntRef", "Will generate instance of int ref type");
        intRefType.add_method("create", "Will generate an int ref", static_cast<int&(GenerateRefTypes<int>::*)(const int&)>(&GenerateRefTypes<int>::operator()));

        Clazz<GenerateRefTypes<const int>> intConstRefType(module, "GenerateIntConstRef", "Will generate instance of int const ref type");
        intConstRefType.add_method("create", "Will generate an int ref", static_cast<const int&(GenerateRefTypes<const int>::*)(const int&)>(&GenerateRefTypes<const int>::operator()));

        Clazz<GenerateRefTypes<double>> doubleRefType(module, "GenerateDoubleRef", "Will generate instance of double ref type");
        doubleRefType.add_method("create", "Will generate an double ref", static_cast<double&(GenerateRefTypes<double>::*)(const double&)>(&GenerateRefTypes<double>::operator()));

        Clazz<GenerateRefTypes<const double>> doubleConstRefType(module, "GenerateDoubleConstRef", "Will generate instance of double const ref type");
        doubleConstRefType.add_method("create", "Will generate an double ref", static_cast<const double&(GenerateRefTypes<const double>::*)(const double&)>(&GenerateRefTypes<const double>::operator()));

        Clazz<GenerateRefTypes<std::string>> strRefType(module, "GenerateStrRef", "Will generate instance of string ref type");
        strRefType.add_method("create", "Will generate an str ref", static_cast<std::string&(GenerateRefTypes<std::string>::*)(const std::string&)>(&GenerateRefTypes<std::string>::operator()));

        Clazz<GenerateRefTypes<const std::string>> strConstRefType(module, "GenerateConstStrRef", "Will generate instance of const string ref type");
        strConstRefType.add_method("create", "Will generate an const str ref", static_cast<const std::string&(GenerateRefTypes<const std::string>::*)(const std::string&)>(&GenerateRefTypes<const std::string>::operator()));

        Clazz<GenerateRefTypes<char*>> ctypeStringRefType(module, "GenerateCTypeStringRef", "Will generate instance of c-type string ref type");
        ctypeStringRefType.add_method("create", "Will generate c-type string ref", static_cast<char*&(GenerateRefTypes<char*>::*)(char*)>(&GenerateRefTypes<char*>::operator()));

        Clazz<GenerateRefTypes<const sweetPy::DateTime>> dateTimeConstRefType(module, "GenerateDateTimeConstRef", "Will generate instance of datetime const ref type");
        dateTimeConstRefType.add_method("create", "Will generate datetime const ref", static_cast<const sweetPy::DateTime&(GenerateRefTypes<const sweetPy::DateTime>::*)(const sweetPy::DateTime&)>(&GenerateRefTypes<const sweetPy::DateTime>::operator()));

        Clazz<GenerateRefTypes<const sweetPy::TimeDelta>> timeDeltaConstRefType(module, "GenerateTimeDeltaConstRef", "Will generate instance of timedelta const ref type");
        timeDeltaConstRefType.add_method("create", "Will generate timedelta const ref", static_cast<const sweetPy::TimeDelta&(GenerateRefTypes<const sweetPy::TimeDelta>::*)(const sweetPy::TimeDelta&)>(&GenerateRefTypes<const sweetPy::TimeDelta>::operator()));
        
        Clazz<GenerateRefTypes<const sweetPy::Dictionary>> dictConstRefType(module, "GenerateDictConstRef", "Will generate instance of dict const ref type");
        dictConstRefType.add_method("create", "Will generate dict const ref", static_cast<const sweetPy::Dictionary&(GenerateRefTypes<const sweetPy::Dictionary>::*)(const sweetPy::Dictionary&)>(&GenerateRefTypes<const sweetPy::Dictionary>::operator()));

        Clazz<GenerateRefTypes<const sweetPy::Tuple>> tupleConstRefType(module, "GenerateTupleConstRef", "Will generate instance of tuple const ref type");
        tupleConstRefType.add_method("create", "Will generate tuple const ref", static_cast<const sweetPy::Tuple&(GenerateRefTypes<const sweetPy::Tuple>::*)(const sweetPy::Tuple&)>(&GenerateRefTypes<const sweetPy::Tuple>::operator()));
        
        Clazz<GenerateRefTypes<const sweetPy::List>> listConstRefType(module, "GenerateListConstRef", "Will generate instance of list const ref type");
        listConstRefType.add_method("create", "Will generate list const ref", static_cast<const sweetPy::List&(GenerateRefTypes<const sweetPy::List>::*)(const sweetPy::List&)>(&GenerateRefTypes<const sweetPy::List>::operator()));
        
        Clazz<GenerateRefTypes<const sweetPy::AsciiString>> asciiStringConstRefType(module, "GenerateAsciiStringConstRef", "Will generate instance of asciistr const ref type");
        asciiStringConstRefType.add_method("create", "Will generate asciistr const ref", static_cast<const sweetPy::AsciiString&(GenerateRefTypes<const sweetPy::AsciiString>::*)(const sweetPy::AsciiString&)>(&GenerateRefTypes<const sweetPy::AsciiString>::operator()));
        
        Clazz<GenerateRefTypes<const sweetPy::ObjectPtr>> objectPtrConstRefType(module, "GenerateObjectPtrConstRef", "Will generate instance of ObjectPtr const ref type");
        objectPtrConstRefType.add_method("create", "Will generate ObjectPtr const ref", static_cast<const sweetPy::ObjectPtr&(GenerateRefTypes<const sweetPy::ObjectPtr>::*)(const sweetPy::ObjectPtr&)>(&GenerateRefTypes<const sweetPy::ObjectPtr>::operator()));
        
        Clazz<GenerateRefTypes<const char*>> ctypestrConstRefType(module, "GenerateCTypeStrConstRef", "Will generate instance of c-type str const ref type");
        ctypestrConstRefType.add_method("create", "Will generate c-type str const ref", static_cast<const char*&(GenerateRefTypes<const char*>::*)(const char*&)>(&GenerateRefTypes<const char*>::operator()));
        
        module.add_function("globalFunction", "global function", &globalFunction);
        
        module.add_function("check_int_conversion", "check integral int type conversions", static_cast<int(*)(int)>(&CheckIntegralIntType));
        module.add_function("check_const_ref_int_conversion", "check integral const ref int type conversions", static_cast<const int&(*)(const int&)>(&CheckIntegralIntType));
        module.add_function("check_ref_int_conversion", "check integral ref int type conversions", static_cast<int&(*)(int&)>(&CheckIntegralIntType));
        module.add_function("check_rvalue_ref_int_conversion", "check integral rvalue ref int type conversions", static_cast<void(*)(int&&)>(&CheckIntegralIntType));
        module.add_function("check_double_conversion", "check integral double type conversions", static_cast<double(*)(double)>(&CheckIntegralDoubleType));
        module.add_function("check_const_ref_double_conversion", "check integral const ref double type conversions", static_cast<const double&(*)(const double&)>(&CheckIntegralDoubleType));
        module.add_function("check_ref_double_conversion", "check integral ref double type conversions", static_cast<double&(*)(double&)>(&CheckIntegralDoubleType));
        module.add_function("check_rvalue_ref_double_conversion", "check integral rvalue ref double type conversions", static_cast<void(*)(double&&)>(&CheckIntegralDoubleType));
        module.add_function("check_str_conversion", "check integral string type conversions", static_cast<std::string(*)(std::string)>(&CheckIntegralStringType));
        module.add_function("check_const_ref_str_conversion", "check integral const ref string type conversions", static_cast<const std::string&(*)(const std::string&)>(&CheckIntegralStringType));
        module.add_function("check_ref_str_conversion", "check integral ref string type conversions", static_cast<std::string&(*)(std::string&)>(&CheckIntegralStringType));
        module.add_function("check_rvalue_ref_str_conversion", "check integral rvalue ref string type conversions", static_cast<void(*)(std::string&&)>(&CheckIntegralStringType));
        module.add_function("check_ref_chararray_conversion", "check integral char array type conversions", static_cast<char(&(*)(char(&)[100]))[100]>(&CheckIntegralCharArrayType));
        module.add_function("check_ctype_string_conversion", "check integral ctype string type conversions", static_cast<void(*)(char*)>(&CheckIntegralCTypeStringType));
        module.add_function("check_const_ctype_string_conversion", "check integral ctype string type conversions", static_cast<const char*(*)(const char*)>(&CheckIntegralConstCTypeStringType));
        module.add_function("check_const_ref_ctype_string_conversion", "check integral ctype string ref type conversions", static_cast<const char*&(*)(const char*&)>(&CheckIntegralConstRefCTypeStrType));
        module.add_function("check_pyobject_conversion", "check integral PyObject type conversions", static_cast<PyObject*(*)(PyObject*)>(&CheckIntegralPyObjectType));
        module.add_function("check_datetime_conversion", "check DateTime type conversions", static_cast<sweetPy::DateTime(*)(sweetPy::DateTime)>(&CheckDateTimeType));
        module.add_function("check_const_ref_datetime_conversion", "check const DateTime& type conversions", static_cast<const sweetPy::DateTime&(*)(const sweetPy::DateTime&)>(&CheckConstRefDateTimeType));
        module.add_function("check_timedelta_conversion", "check TimeDelta type conversions", static_cast<sweetPy::TimeDelta(*)(sweetPy::TimeDelta)>(&CheckTimeDeltaType));
        module.add_function("check_const_ref_timedelta_conversion", "check const TimeDelta& type conversions", static_cast<const sweetPy::TimeDelta&(*)(const sweetPy::TimeDelta&)>(&CheckConstRefTimeDeltaType));
        module.add_function("check_dict_conversion", "check Dictionary type conversions", static_cast<sweetPy::Dictionary(*)(sweetPy::Dictionary)>(&CheckDictionaryType));
        module.add_function("check_tuple_conversion", "check Tuple type conversions", static_cast<sweetPy::Tuple(*)(sweetPy::Tuple)>(&CheckTuleType));
        module.add_function("check_const_ref_tuple_conversion", "check const Tuple& type conversions", static_cast<const sweetPy::Tuple&(*)(const sweetPy::Tuple&)>(&CheckConstRefTupleType));
        module.add_function("generate_native_element_tuple", "Will generate a native tuple which will hold non supported element type", static_cast<sweetPy::Tuple(*)()>(&GenerateNativeElementTuple));
        module.add_function("check_list_conversion", "check List type conversions", static_cast<sweetPy::List(*)(sweetPy::List)>(&CheckListType));
        module.add_function("check_const_ref_list_conversion", "check const List& type conversions", static_cast<const sweetPy::List&(*)(const sweetPy::List&)>(&CheckConstRefListType));
        module.add_function("generate_native_element_list", "Will generate a native list which will hold non supported element type", static_cast<sweetPy::List(*)()>(&GenerateNativeElementList));
        module.add_function("check_asciistr_conversion", "check AsciiString type conversions", static_cast<sweetPy::AsciiString(*)(sweetPy::AsciiString)>(&CheckAsciiStringType));
        module.add_function("check_const_ref_asciistr_conversion", "check const AsciiString& type conversions", static_cast<const sweetPy::AsciiString&(*)(const sweetPy::AsciiString&)>(&CheckConstRefAsciiStringType));
        module.add_function("check_objectptr_conversion", "check ObjectPtr type conversions", static_cast<sweetPy::ObjectPtr(*)(sweetPy::ObjectPtr)>(&CheckObjectPtrType));
        module.add_function("check_const_ref_objectptr_conversion", "check const ObjectPtr& type conversions", static_cast<const sweetPy::ObjectPtr&(*)(const sweetPy::ObjectPtr&)>(&CheckConstRefObjectPtrType));
        
        //Integral types (lvalue, rvalue, const modifier)
        
        const int var = 6;
        TestSubjectB b;
        module.add_variable("globalVariableLiteral", "Hello World");
        module.add_variable("globalVariableInt_rvalue", 5);
        module.add_variable("globalVariableInt_lvalue_const", var);
        //standard user type, which is supported
        module.add_variable("globalVariableStr_rvalue", std::string("Hello World"));
        //Non supported user type
        module.add_variable("globalVariableUserType_rvalue", TestSubjectB());
        module.add_variable("globalVariableUserType_lvalue", b);
    }

}
