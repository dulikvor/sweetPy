#include <Python.h>
#include <type_traits>
#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "core/Logger.h"
#include "PythonEmbedder.h"
#include "CPythonClassTestModule.h"
#include "Core/PythonAssist.h"
#include "Utility/Types_generated.h"
#include "Utility/Serialize.h"

static int _argc;
static char **_argv;

namespace sweetPyTest {

    class CPythonClassTest : public ::testing::Environment {
    public:
        CPythonClassTest() {}

        void SetUp() override {
            core::Logger::Instance().Start(core::TraceSeverity::Info);
            PythonEmbedder::Instance().InitiateInterperter("CPythonClassTest", _argc, _argv);
            const char *testingScript = "from CPythonClassTestModule import TestClass, Enum_Python, TestClassB, TestClassC, globalFunction, globalVariableLiteral, globalVariableInt_rvalue, globalVariableInt_lvalue_const, globalVariableStr_rvalue, globalVariableUserType_rvalue, globalVariableUserType_lvalue\n"
                                        "import CPythonClassTestModule as TestModule";
            PyRun_SimpleString(testingScript);
        }

        void TearDown() override {
            PythonEmbedder::Instance().TerminateInterperter();
        }
    };

    //Test is going to validate at compile time the correctness of different types use cases
    // being handled by ObjectWrapper and Object.
    TEST(CPythonClassTest, CPythonObjectWrapperTest){
        {
            //The use case of non copyable/moveable reference type
            static_assert(std::is_same<typename sweetPy::ObjectWrapper<TestSubjectC&,0>::Type, void*>::value, "ObjectWrapper Type - validating a non copyable/moveable reference type assertion has failed.");
            static_assert(std::is_same<typename sweetPy::ObjectWrapper<TestSubjectC&,0>::FromPythonType, PyObject*>::value, "ObjectWrapper FromPython - validating a non copyable/moveable reference type assertion has failed.");
        }
    }

    TEST(CPythonClassTest, CPythonObjectCheckIntIntegralType)
    {
        const char *testingScript = "intArgument = 1000\n"
                "intReturn = TestModule.check_int_conversion(intArgument) #PyLong -> int\n"
                "intRefGenerator = TestModule.GenerateIntRef()\n"
                "intRefObject = intRefGenerator.create(500)\n"
                "intReturn_2 = TestModule.check_int_conversion(intRefObject) #ref int -> int\n"
                "intConstRefGenerator = TestModule.GenerateIntConstRef()\n"
                "intConstRefObject = intConstRefGenerator.create(501)\n"
                "intReturn_3 = TestModule.check_int_conversion(intConstRefObject) #ref const int -> int\n"
                "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                "intArgument_2 = 1000\n"
                "intConstRefObject_2 = TestModule.check_const_ref_int_conversion(intArgument_2) #PyLong -> const int&\n"
                "intRefObject_2 = intRefGenerator.create(500)\n"
                "intConstRefObject_3 = TestModule.check_const_ref_int_conversion(intRefObject_2) #ref int -> const int&\n"
                "intConstRefObject_4 = intConstRefGenerator.create(501)\n"
                "intConstRefObject_5 = TestModule.check_const_ref_int_conversion(intConstRefObject_4) #const ref int -> const int&\n"
                "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                "intRefObject_3 = intRefGenerator.create(501)\n"
                "intRefObject_4 = TestModule.check_ref_int_conversion(intRefObject_3) #ref int -> int&";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<int>("intArgument"));
        ASSERT_EQ(1001, PythonEmbedder::GetAttribute<int>("intReturn"));
        ASSERT_EQ(500, PythonEmbedder::GetAttribute<int&>("intRefObject"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<int>("intReturn_2"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<const int&>("intConstRefObject"));
        ASSERT_EQ(502, PythonEmbedder::GetAttribute<int>("intReturn_3"));

        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<int>("intArgument_2"));
        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<const int&>("intConstRefObject_2"));
        ASSERT_EQ(500, PythonEmbedder::GetAttribute<int&>("intRefObject_2"));
        ASSERT_EQ(500, PythonEmbedder::GetAttribute<const int&>("intConstRefObject_3"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<const int&>("intConstRefObject_4"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<const int&>("intConstRefObject_5"));

        ASSERT_EQ(502, PythonEmbedder::GetAttribute<int&>("intRefObject_3"));
        ASSERT_EQ(502, PythonEmbedder::GetAttribute<int&>("intRefObject_4"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckDoubleIntegralType)
    {
        const char *testingScript = "intArgument = 1000\n"
                "doubleReturn = TestModule.check_double_conversion(intArgument) #PyLong -> double\n"
                "doubleArgument = 1000.5\n"
                "doubleReturn_2 = TestModule.check_double_conversion(doubleArgument) #PyFloat -> double\n"
                "intRefGenerator = TestModule.GenerateIntRef()\n"
                "intRefObject = intRefGenerator.create(500)\n"
                "doubleReturn_3 = TestModule.check_double_conversion(intRefObject) #ref int -> double\n"
                "intConstRefGenerator = TestModule.GenerateIntConstRef()\n"
                "intConstRefObject = intConstRefGenerator.create(501)\n"
                "doubleReturn_4 = TestModule.check_double_conversion(intConstRefObject) #ref const int -> double\n"
                "doubleRefGenerator = TestModule.GenerateDoubleRef()\n"
                "doubleRefObject = doubleRefGenerator.create(500.5)\n"
                "doubleReturn_5 = TestModule.check_double_conversion(doubleRefObject) #ref double-> double\n"
                "doubleConstRefGenerator = TestModule.GenerateDoubleConstRef()\n"
                "doubleConstRefObject = doubleConstRefGenerator.create(501.5)\n"
                "doubleReturn_6 = TestModule.check_double_conversion(doubleConstRefObject) #ref const double -> double\n"
                "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                "intArgument_2 = 1000\n"
                "doubleConstRefObject_2 = TestModule.check_const_ref_double_conversion(intArgument_2) #PyLong -> const double&\n"
                "doubleArgument_2 = 1000.5\n"
                "doubleConstRefObject_3 = TestModule.check_const_ref_double_conversion(doubleArgument_2) #PyFloat -> const double&\n"
                "doubleRefObject_2 = doubleRefGenerator.create(500.5)\n"
                "doubleConstRefObject_4 = TestModule.check_const_ref_double_conversion(doubleRefObject_2) #ref double -> const int&\n"
                "doubleConstRefObject_5 = doubleConstRefGenerator.create(501.5)\n"
                "doubleConstRefObject_6 = TestModule.check_const_ref_double_conversion(doubleConstRefObject_5) #const ref double -> const int&\n"
                "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                "doubleRefObject_3 = doubleRefGenerator.create(502.35)\n"
                "doubleRefObject_4 = TestModule.check_ref_double_conversion(doubleRefObject_3) #ref double -> double&";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<int>("intArgument"));
        ASSERT_EQ(1001, PythonEmbedder::GetAttribute<double>("doubleReturn"));
        ASSERT_EQ(1000.5, PythonEmbedder::GetAttribute<double>("doubleArgument"));
        ASSERT_EQ(1001.5, PythonEmbedder::GetAttribute<double>("doubleReturn_2"));
        ASSERT_EQ(500, PythonEmbedder::GetAttribute<int&>("intRefObject"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<double>("doubleReturn_3"));
        ASSERT_EQ(501, PythonEmbedder::GetAttribute<const int&>("intConstRefObject"));
        ASSERT_EQ(502, PythonEmbedder::GetAttribute<double>("doubleReturn_4"));
        ASSERT_EQ(500.5, PythonEmbedder::GetAttribute<double&>("doubleRefObject"));
        ASSERT_EQ(501.5, PythonEmbedder::GetAttribute<double>("doubleReturn_5"));
        ASSERT_EQ(501.5, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject"));
        ASSERT_EQ(502.5, PythonEmbedder::GetAttribute<double>("doubleReturn_6"));

        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<int>("intArgument_2"));
        ASSERT_EQ(1000, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject_2"));
        ASSERT_EQ(1000.5, PythonEmbedder::GetAttribute<double>("doubleArgument_2"));
        ASSERT_EQ(1000.5, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject_3"));
        ASSERT_EQ(500.5, PythonEmbedder::GetAttribute<double&>("doubleRefObject_2"));
        ASSERT_EQ(500.5, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject_4"));
        ASSERT_EQ(501.5, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject_5"));
        ASSERT_EQ(501.5, PythonEmbedder::GetAttribute<const double&>("doubleConstRefObject_6"));

        ASSERT_EQ(503.35, PythonEmbedder::GetAttribute<double&>("doubleRefObject_3"));
        ASSERT_EQ(503.35, PythonEmbedder::GetAttribute<double&>("doubleRefObject_4"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckStringIntegralType)
    {
        const char *testingScript = "strArgument = 'hello'\n"
                                    "strReturn = TestModule.check_str_conversion(strArgument) #Unicode string -> std::string\n"
                                    "bytesArgument = b'hello'\n"
                                    "strReturn_2 = TestModule.check_str_conversion(bytesArgument) #Bytes array -> std::string\n"
                                    "strRefGenerator = TestModule.GenerateStrRef()\n"
                                    "strRefObject = strRefGenerator.create('hello')\n"
                                    "strReturn_3 = TestModule.check_str_conversion(strRefObject) #std::string& -> std::string\n"
                                    "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                                    "strRefObject_2 = strRefGenerator.create('hello')\n"
                                    "strRefReturn = TestModule.check_ref_str_conversion(strRefObject_2) #std::string& -> std::string&\n"
                                    "'''++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'''\n"
                                    "strArgument_2 = 'hello'\n"
                                    "strConstRefReturn = TestModule.check_const_ref_str_conversion(strArgument_2) #Unicode string -> const std::string&\n"
                                    "bytesArgument_2 = b'hello'\n"
                                    "strConstRefReturn_2 = TestModule.check_const_ref_str_conversion(bytesArgument_2) #Bytes array -> const std::string&\n"
                                    "strRefObject_3 = strRefGenerator.create('hello')\n"
                                    "strConstRefReturn_3 = TestModule.check_const_ref_str_conversion(strRefObject_3) #std::string& -> const std::string&\n"
                                    "strConstRefGenerator = TestModule.GenerateConstStrRef()\n"
                                    "strConstRefObject = strConstRefGenerator.create('hello')\n"
                                    "strConstRefReturn_4 = TestModule.check_const_ref_str_conversion(strConstRefObject) #const std::string& -> const std::string&\n";

        PyRun_SimpleString(testingScript);
        //std:string
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string>("strArgument"));
        ASSERT_EQ("hello world", PythonEmbedder::GetAttribute<std::string>("strReturn"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string>("bytesArgument"));
        ASSERT_EQ("hello world", PythonEmbedder::GetAttribute<std::string>("strReturn_2"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string&>("strRefObject"));
        ASSERT_EQ("hello world", PythonEmbedder::GetAttribute<std::string>("strReturn_3"));
        //std::string&
        ASSERT_EQ("hello to all", PythonEmbedder::GetAttribute<std::string&>("strRefObject_2"));
        ASSERT_EQ("hello to all", PythonEmbedder::GetAttribute<std::string>("strRefReturn"));
        //const std::string&
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string>("strArgument_2"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<const std::string&>("strConstRefReturn"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string>("bytesArgument_2"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<const std::string&>("strConstRefReturn_2"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<std::string&>("strRefObject_3"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<const std::string&>("strConstRefReturn_3"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<const std::string&>("strConstRefObject"));
        ASSERT_EQ("hello", PythonEmbedder::GetAttribute<const std::string&>("strConstRefReturn_4"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckCTypeStringIntegralType)
    {
        const char *testingScript = "bytesArgument = b'hello'\n"
                "TestModule.check_ctype_string_conversion(bytesArgument) #Bytes array -> char*\n"
                "ctypeStringRefGenerator = TestModule.GenerateCTypeStringRef()\n"
                "ctypeStringRefObject = ctypeStringRefGenerator.create(b'gelo')\n"
                "TestModule.check_ctype_string_conversion(ctypeStringRefObject) #char*& -> std::string\n";

        PyRun_SimpleString(testingScript);
        //char*
        ASSERT_EQ(strcmp("lello",PythonEmbedder::GetAttribute<char*>("bytesArgument")), 0);
        ASSERT_EQ(strcmp("lelo",PythonEmbedder::GetAttribute<char*>("ctypeStringRefObject")), 0);
    }


    TEST(CPythonClassTest, CPythonObjectCheckPyObjectIntegralType)
    {
    const char *testingScript = "pyObjectArgument = 5\n"
                                "pyObjectReturn = TestModule.check_pyobject_conversion(pyObjectArgument) #PyObject* -> PyObject*\n";

    PyRun_SimpleString(testingScript);
    ASSERT_EQ(PythonEmbedder::GetAttribute<PyObject*>("pyObjectReturn"), PythonEmbedder::GetAttribute<PyObject*>("pyObjectArgument"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckDateTimeType)
    {
        const char *testingScript = "from datetime import datetime\n"
                                    //DateTime
                                    "datetimeArgument = datetime(2018, 1, 1, hour = 9, minute = 8, second = 7, microsecond = 6)\n"
                                    "datetimeReturn = TestModule.check_datetime_conversion(datetimeArgument) #datetime.datetime -> DateTime\n"
                                    "datetimeConstRefGenerator = TestModule.GenerateDateTimeConstRef()\n"
                                    "datetimeArgument_2 = datetime(2018, 1, 1, hour = 9, minute = 8, second = 7, microsecond = 6)\n"
                                    "datetimeConstRefObject = datetimeConstRefGenerator.create(datetimeArgument_2)\n"
                                    "datetimeReturn_2 = TestModule.check_datetime_conversion(datetimeConstRefObject) #const DateTime& -> DateTime\n"
                                    //const DateTime&
                                    "datetimeArgument_3 = datetime(2018, 1, 1, hour = 9, minute = 8, second = 7, microsecond = 6)\n"
                                    "datetimeReturn_3 = TestModule.check_const_ref_datetime_conversion(datetimeArgument_3) #datetime.datetime -> const DateTime&\n"
                                    "datetimeArgument_4 = datetime(2018, 1, 1, hour = 9, minute = 8, second = 7, microsecond = 6)\n"
                                    "datetimeConstRefObject_2 = datetimeConstRefGenerator.create(datetimeArgument_4)\n"
                                    "datetimeReturn_4 = TestModule.check_const_ref_datetime_conversion(datetimeConstRefObject_2) #const DateTime& -> const DateTime&";

        PyRun_SimpleString(testingScript);
        //DateTime
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::DateTime>("datetimeArgument"));
        ASSERT_EQ(sweetPy::DateTime(1, 2, 3, 4, 5), PythonEmbedder::GetAttribute<sweetPy::DateTime>("datetimeReturn"));
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::DateTime&>("datetimeConstRefObject"));
        ASSERT_EQ(sweetPy::DateTime(1, 2, 3, 4, 5), PythonEmbedder::GetAttribute<sweetPy::DateTime>("datetimeReturn_2"));
        //const DateTime&
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::DateTime>("datetimeArgument_3"));
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::DateTime&>("datetimeReturn_3"));
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::DateTime>("datetimeArgument_4"));
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::DateTime&>("datetimeConstRefObject_2"));
        ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::DateTime&>("datetimeReturn_4"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckTimeDeltaType)
    {
        const char *testingScript = "from datetime import timedelta\n"
                //TimeDelta
                "timedeltaArgument = timedelta(seconds = 7, microseconds = 6)\n"
                "timedeltaReturn = TestModule.check_timedelta_conversion(timedeltaArgument) #datetime.timedelta -> TimeDelta\n"
                "timedeltaConstRefGenerator = TestModule.GenerateTimeDeltaConstRef()\n"
                "timedeltaArgument_2 = timedelta(seconds = 7, microseconds = 6)\n"
                "timedeltaConstRefObject = timedeltaConstRefGenerator.create(timedeltaArgument_2)\n"
                "timedeltaReturn_2 = TestModule.check_timedelta_conversion(timedeltaConstRefObject) #const TimeDelta& -> TimeDelta\n"
                //const TimeDelta&
                "timedeltaArgument_3 = timedelta(seconds = 7, microseconds = 6)\n"
                "timedeltaReturn_3 = TestModule.check_const_ref_timedelta_conversion(timedeltaArgument_3) #datetime.timedelta -> const TimeDelta&\n"
                "timedeltaArgument_4 = timedelta(seconds = 7, microseconds = 6)\n"
                "timedeltaConstRefObject_2 = timedeltaConstRefGenerator.create(timedeltaArgument_4)\n"
                "timedeltaReturn_4 = TestModule.check_const_ref_timedelta_conversion(timedeltaConstRefObject_2) #const TimeDelta& -> const TimeDelta&";

        PyRun_SimpleString(testingScript);
        //TimeDelta
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::TimeDelta>("timedeltaArgument"));
        ASSERT_EQ(sweetPy::TimeDelta(3, 4, 5), PythonEmbedder::GetAttribute<sweetPy::TimeDelta>("timedeltaReturn"));
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::TimeDelta&>("timedeltaConstRefObject"));
        ASSERT_EQ(sweetPy::TimeDelta(3, 4, 5), PythonEmbedder::GetAttribute<sweetPy::TimeDelta>("timedeltaReturn_2"));
        //const TimeDelta&
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::TimeDelta>("timedeltaArgument_3"));
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::TimeDelta&>("timedeltaReturn_3"));
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<sweetPy::TimeDelta>("timedeltaArgument_4"));
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::TimeDelta&>("timedeltaConstRefObject_2"));
        ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::GetAttribute<const sweetPy::TimeDelta&>("timedeltaReturn_4"));
    }

    TEST(CPythonClassTest, CPythonObjectCheckTupleType)
    {

        const char *testingScript = "from datetime import timedelta\n"
                //Tuple
                "tupleArgument = (1, 1.2, 'hello', True)\n"
                "tupleReturn = TestModule.check_tuple_conversion(tupleArgument) #tuple -> Tuple\n"
                "tupleConstRefGenerator = TestModule.GenerateTupleConstRef()\n"
                "tupleArgument_2 = (5, 7.2)\n"
                "tupleConstRefObject = tupleConstRefGenerator.create(tupleArgument_2)\n"
                "tupleReturn_2 = TestModule.check_tuple_conversion(tupleConstRefObject) #const Tuple& -> Tuple\n"
                //const Tuple&
                "tupleArgument_3 = (False, 'to all')\n"
                "tupleReturn_3 = TestModule.check_const_ref_tuple_conversion(tupleArgument_3) #tuple -> const Tuple&\n"
                "tupleArgument_4 = (None,)\n"
                "tupleConstRefObject_2 = tupleConstRefGenerator.create(tupleArgument_4)\n"
                "tupleReturn_4 = TestModule.check_const_ref_tuple_conversion(tupleConstRefObject_2) #const Tuple& -> const Tuple&\n"
                //Transformation of non supported type element into python representation
                "tupleReturn_5 = TestModule.generate_native_element_tuple()\n"
                "value = tupleReturn_5[0]";

        PyRun_SimpleString(testingScript);
        //Tuple
        sweetPy::Tuple tupleArgument = PythonEmbedder::GetAttribute<sweetPy::Tuple>("tupleReturn");
        ASSERT_EQ(1, tupleArgument.GetElement<int>(0));
        ASSERT_EQ(2.5, tupleArgument.GetElement<double>(1));
        ASSERT_EQ(std::string("Goodbye"), tupleArgument.GetElement<std::string>(2));
        ASSERT_EQ(std::string("World"), tupleArgument.GetElement<std::string>(3));
        ASSERT_EQ(true, tupleArgument.GetElement<bool>(4));

        const sweetPy::Tuple& tupleConstRefObject = PythonEmbedder::GetAttribute<const sweetPy::Tuple&>("tupleConstRefObject");
        ASSERT_EQ(5, tupleConstRefObject.GetElement<int>(0));
        ASSERT_EQ(7.2, tupleConstRefObject.GetElement<double>(1));

        sweetPy::Tuple tupleReturn_2 = PythonEmbedder::GetAttribute<sweetPy::Tuple>("tupleReturn_2");
        ASSERT_EQ(1, tupleReturn_2.GetElement<int>(0));
        ASSERT_EQ(2.5, tupleReturn_2.GetElement<double>(1));
        ASSERT_EQ(std::string("Goodbye"), tupleReturn_2.GetElement<std::string>(2));
        ASSERT_EQ(std::string("World"), tupleReturn_2.GetElement<std::string>(3));
        ASSERT_EQ(true, tupleReturn_2.GetElement<bool>(4));
        //const Tuple&
        sweetPy::Tuple tupleArgument_3 = PythonEmbedder::GetAttribute<sweetPy::Tuple>("tupleArgument_3");
        ASSERT_EQ(0, tupleArgument_3.GetElement<bool>(0));
        ASSERT_EQ(std::string("to all"), tupleArgument_3.GetElement<std::string>(1));

        const sweetPy::Tuple& tupleReturn_3 = PythonEmbedder::GetAttribute<const sweetPy::Tuple&>("tupleReturn_3");
        ASSERT_EQ(0, tupleReturn_3.GetElement<bool>(0));
        ASSERT_EQ(std::string("to all"), tupleReturn_3.GetElement<std::string>(1));

        sweetPy::Tuple tupleArgument_4 = PythonEmbedder::GetAttribute<sweetPy::Tuple>("tupleArgument_4");
        ASSERT_EQ(nullptr, tupleArgument_4.GetElement<void*>(0));

        const sweetPy::Tuple& tupleConstRefObject_2 = PythonEmbedder::GetAttribute<const sweetPy::Tuple&>("tupleConstRefObject_2");
        ASSERT_EQ(nullptr, tupleConstRefObject_2.GetElement<void*>(0));

        const sweetPy::Tuple& tupleReturn_4 = PythonEmbedder::GetAttribute<const sweetPy::Tuple&>("tupleReturn_4");
        ASSERT_EQ(nullptr, tupleReturn_4.GetElement<void*>(0));
        //Transformation of non supported type element into python representation
        TestSubjectB& value = PythonEmbedder::GetAttribute<TestSubjectB&>("value");
        ASSERT_EQ(value.GetValue(), 0);
        ASSERT_EQ(value.GetStr(), "Hello World");

        //Check native type
        sweetPy::Tuple tuple;
        //int
        tuple.AddElement(5);
        ASSERT_EQ(5, tuple.GetElement<int>(0));
        int int_val = 6;
        tuple.AddElement(int_val);
        ASSERT_EQ(6, tuple.GetElement<int>(1));
        const int const_int_val = 7;
        tuple.AddElement(const_int_val);
        ASSERT_EQ(7, tuple.GetElement<int>(2));
        tuple.Clear();
        //double
        tuple.AddElement(5.5);
        ASSERT_EQ(5.5, tuple.GetElement<double>(0));
        double double_val = 6.5;
        tuple.AddElement(double_val);
        ASSERT_EQ(6.5, tuple.GetElement<double>(1));
        const double const_double_val = 7.5;
        tuple.AddElement(const_double_val);
        ASSERT_EQ(7.5, tuple.GetElement<double>(2));
        tuple.Clear();
        //bool
        tuple.AddElement(true);
        ASSERT_EQ(true, tuple.GetElement<bool>(0));
        bool bool_val = false;
        tuple.AddElement(bool_val);
        ASSERT_EQ(0, tuple.GetElement<bool>(1));
        const bool const_bool_val = false;
        tuple.AddElement(const_bool_val);
        ASSERT_EQ(0, tuple.GetElement<bool>(2));
        tuple.Clear();
        //Ctype string
        tuple.AddElement("A");
        ASSERT_EQ("A", tuple.GetElement<std::string>(0));
        const char* cstr_val = "B";
        tuple.AddElement(const_cast<char*>(cstr_val));
        ASSERT_EQ("B", tuple.GetElement<std::string>(1));
        const char* const_cstr_val = "C";
        tuple.AddElement(const_cstr_val);
        ASSERT_EQ("C", tuple.GetElement<std::string>(2));
        tuple.Clear();
        //String
        tuple.AddElement(std::string("A"));
        ASSERT_EQ(std::string("A"), tuple.GetElement<std::string>(0));
        std::string str_val = "B";
        tuple.AddElement(str_val);
        ASSERT_EQ(std::string("B"), tuple.GetElement<std::string>(1));
        const std::string const_str_val = "C";
        tuple.AddElement(const_str_val);
        ASSERT_EQ(std::string("C"), tuple.GetElement<std::string>(2));
        tuple.Clear();
        //void*
        tuple.AddElement(nullptr);
        ASSERT_EQ(nullptr, tuple.GetElement<void*>(0));
        void* ptr_val = nullptr;
        tuple.AddElement(ptr_val);
        ASSERT_EQ(nullptr, tuple.GetElement<void*>(1));
        const void* const_ptr_val = nullptr;
        tuple.AddElement(const_ptr_val);
        ASSERT_EQ(nullptr, tuple.GetElement<void*>(2));
        tuple.Clear();
        //Copy constructor
        tuple.AddElement(5);
        tuple.AddElement(5.5);
        tuple.AddElement(true);
        tuple.AddElement("A");
        tuple.AddElement(std::string("B"));
        tuple.AddElement(nullptr);
        sweetPy::Tuple cpy_tuple(tuple);
        ASSERT_EQ(cpy_tuple.GetElement<int>(0), tuple.GetElement<int>(0));
        ASSERT_EQ(cpy_tuple.GetElement<double>(1), tuple.GetElement<double>(1));
        ASSERT_EQ(cpy_tuple.GetElement<bool>(2), tuple.GetElement<bool>(2));
        ASSERT_EQ(cpy_tuple.GetElement<std::string>(3), tuple.GetElement<std::string>(3));
        ASSERT_EQ(cpy_tuple.GetElement<std::string>(4), tuple.GetElement<std::string>(4));
        ASSERT_EQ(cpy_tuple.GetElement<void*>(5), tuple.GetElement<void*>(5));
        tuple.Clear();
        //Copy assigment
        tuple.AddElement(5);
        tuple.AddElement(5.5);
        tuple.AddElement(true);
        tuple.AddElement("A");
        tuple.AddElement(std::string("B"));
        tuple.AddElement(nullptr);
        sweetPy::Tuple cpy_assign_tuple;
        cpy_assign_tuple = tuple;
        ASSERT_EQ(cpy_assign_tuple.GetElement<int>(0), tuple.GetElement<int>(0));
        ASSERT_EQ(cpy_assign_tuple.GetElement<double>(1), tuple.GetElement<double>(1));
        ASSERT_EQ(cpy_assign_tuple.GetElement<bool>(2), tuple.GetElement<bool>(2));
        ASSERT_EQ(cpy_assign_tuple.GetElement<std::string>(3), tuple.GetElement<std::string>(3));
        ASSERT_EQ(cpy_assign_tuple.GetElement<std::string>(4), tuple.GetElement<std::string>(4));
        ASSERT_EQ(cpy_assign_tuple.GetElement<void*>(5), tuple.GetElement<void*>(5));
        tuple.Clear();
        //Move constructor
        tuple.AddElement(5);
        tuple.AddElement(5.5);
        tuple.AddElement(true);
        tuple.AddElement("A");
        tuple.AddElement(std::string("B"));
        tuple.AddElement(nullptr);
        sweetPy::Tuple mv_tuple(std::move(tuple));
        ASSERT_EQ(mv_tuple.GetElement<int>(0), 5);
        ASSERT_EQ(mv_tuple.GetElement<double>(1), 5.5);
        ASSERT_EQ(mv_tuple.GetElement<bool>(2), true);
        ASSERT_EQ(mv_tuple.GetElement<std::string>(3), std::string("A"));
        ASSERT_EQ(mv_tuple.GetElement<std::string>(4), std::string("B"));
        ASSERT_EQ(mv_tuple.GetElement<void*>(5), nullptr);
        tuple.Clear();
        //Move assigment
        tuple.AddElement(5);
        tuple.AddElement(5.5);
        tuple.AddElement(true);
        tuple.AddElement("A");
        tuple.AddElement(std::string("B"));
        tuple.AddElement(nullptr);
        sweetPy::Tuple mv_assign_tuple;
        mv_assign_tuple = std::move(tuple);
        ASSERT_EQ(mv_assign_tuple.GetElement<int>(0), 5);
        ASSERT_EQ(mv_assign_tuple.GetElement<double>(1), 5.5);
        ASSERT_EQ(mv_assign_tuple.GetElement<bool>(2), true);
        ASSERT_EQ(mv_assign_tuple.GetElement<std::string>(3), std::string("A"));
        ASSERT_EQ(mv_assign_tuple.GetElement<std::string>(4), std::string("B"));
        ASSERT_EQ(mv_assign_tuple.GetElement<void*>(5), nullptr);
        tuple.Clear();
    }
    
    TEST(CPythonClassTest, CPythonObjectCheckAsciiStringType)
    {
        const char *testingScript = "unicodeArgument = 'Hello World'\n"
                                    "asciistrReturn = TestModule.check_asciistr_conversion(unicodeArgument) #unicode -> AsciiString\n"
                                    "asciistrConstRefGenerator = TestModule.GenerateAsciiStringConstRef()\n"
                                    "asciistrArgument_2 = 'Justice to all'\n"
                                    "asciistrConstRefObject = asciistrConstRefGenerator.create(asciistrArgument_2)\n"
                                    "asciistrReturn_2 = TestModule.check_asciistr_conversion(asciistrConstRefObject) #const AsciiString& -> AsciiString\n"
                                    //const TimeDelta&
                                    "asciistrArgument_3 = 'Death magnetic is not bad'\n"
                                    "asciistrReturn_3 = TestModule.check_const_ref_asciistr_conversion(asciistrArgument_3) #unicode -> const AsciiString&\n"
                                    "asciistrArgument_4 = 'Lulu? come on?!'\n"
                                    "asciistrConstRefObject_2 = asciistrConstRefGenerator.create(asciistrArgument_4)\n"
                                    "asciistrReturn_4 = TestModule.check_const_ref_asciistr_conversion(asciistrConstRefObject_2) #const AsciiString& -> const AsciiString&";
        
        PyRun_SimpleString(testingScript);
        //TimeDelta
        ASSERT_EQ(std::string("Babylon 5 Rulezzzzz!"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<sweetPy::AsciiString>("asciistrReturn")));
        ASSERT_EQ(std::string("Justice to all"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<const sweetPy::AsciiString&>("asciistrConstRefObject")));
        ASSERT_EQ(std::string("Babylon 5 Rulezzzzz!"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<sweetPy::AsciiString>("asciistrReturn_2")));
        //const TimeDelta&
        ASSERT_EQ(std::string("Death magnetic is not bad"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<const sweetPy::AsciiString&>("asciistrReturn_3")));
        ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<const sweetPy::AsciiString&>("asciistrConstRefObject_2")));
        ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<const std::string&>(PythonEmbedder::GetAttribute<const sweetPy::AsciiString&>("asciistrReturn_4")));
        
        //Native
        sweetPy::AsciiString str("Lulu? come on?!");
        ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<const std::string&>(str));
        ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<std::string>(str));
    }
    
    TEST(CPythonClassTest, CPythonObjectCheckConstCTypeSrType)
    {
        const char *testingScript = "pyUnicodeArgument = 'Make love'\n"
                                    "constCTypeStrReturn = TestModule.check_const_ctype_string_conversion(pyUnicodeArgument) #PyUniCode -> const char*\n"
                                    "ctypestrConstRefGenerator = TestModule.GenerateCTypeStrConstRef()\n"
                                    "pyUnicodeArgument_2 = 'Fever is no joke'\n"
                                    "constCTypeStrReturn_2 = ctypestrConstRefGenerator.create(pyUnicodeArgument_2)\n"
                                    "constCTypeStrReturn_3 = TestModule.check_const_ctype_string_conversion(constCTypeStrReturn_2) #const char*& -> const char*\n"
                                    "pyUnicodeArgument_3 = b'Long live Sheridan'\n"
                                    "constCTypeStrReturn_4 = TestModule.check_const_ctype_string_conversion(pyUnicodeArgument_3) #PyBytes -> const char*\n"
                                    "constCTypeStrReturn_5 = TestModule.check_const_ref_ctype_string_conversion(pyUnicodeArgument) #PyUnicode -> const char*&\n"
                                    "constCTypeStrReturn_6 = TestModule.check_const_ref_ctype_string_conversion(pyUnicodeArgument_3) #PyBytes -> const char*&\n"
                                    "constCTypeStrReturn_7 = TestModule.check_const_ref_ctype_string_conversion(constCTypeStrReturn_2) #const char*& -> const char*&\n";
        
        PyRun_SimpleString(testingScript);
        //const char*
        ASSERT_EQ("Make love", PythonEmbedder::GetAttribute<std::string>("constCTypeStrReturn"));
        ASSERT_EQ("Fever is no joke", PythonEmbedder::GetAttribute<std::string>("constCTypeStrReturn_3"));
        ASSERT_EQ("Long live Sheridan", PythonEmbedder::GetAttribute<std::string>("constCTypeStrReturn_4"));
        //const char*&
        ASSERT_EQ(std::string(PythonEmbedder::GetAttribute<const char*>("constCTypeStrReturn_5")), "Make love");
        ASSERT_EQ(std::string(PythonEmbedder::GetAttribute<const char*>("constCTypeStrReturn_6")), "Long live Sheridan");
        ASSERT_EQ(std::string(PythonEmbedder::GetAttribute<const char*>("constCTypeStrReturn_7")), "Fever is no joke");
    }
    
    TEST(CPythonClassTest, CPythonObjectCheckObjectPtrType)
    {
        const char *testingScript = "pyLongArgument = 5\n"
                                    "objectptrReturn = TestModule.check_objectptr_conversion(pyLongArgument) #PyLong -> object_ptr\n"
                                    //const object_ptr&
                                    "objectptrConstRefGenerator = TestModule.GenerateObjectPtrConstRef()\n"
                                    "pyLongArgument_2 = 3\n"
                                    "objectptrReturn_2 = TestModule.check_const_ref_objectptr_conversion(pyLongArgument_2) #const object_ptr& -> object_ptr\n"
                                    "pyLongArgument_3 = 4\n"
                                    "objectptrConstRefObject = objectptrConstRefGenerator.create(pyLongArgument_3)\n"
                                    "objectptrReturn_3 = TestModule.check_const_ref_objectptr_conversion(objectptrConstRefObject) #const object_ptr& -> object_ptr\n";
        
        PyRun_SimpleString(testingScript);
        //object_ptr
        ASSERT_EQ(6, PythonEmbedder::GetAttribute<int>("objectptrReturn"));
        //const object_ptr&
        ASSERT_EQ(4, sweetPy::Object<int>::FromPython(static_cast<const sweetPy::object_ptr&>(PythonEmbedder::GetAttribute<const sweetPy::object_ptr&>("objectptrReturn_2")).get()));
        ASSERT_EQ(5, sweetPy::Object<int>::FromPython(static_cast<const sweetPy::object_ptr&>(PythonEmbedder::GetAttribute<const sweetPy::object_ptr&>("objectptrReturn_3")).get()));
        auto& generator = PythonEmbedder::GetAttribute<GenerateRefTypes<const sweetPy::object_ptr>&>("objectptrConstRefGenerator");
        generator.Clear();
    }

    TEST(CPythonClassTest, PythonFunctionInvocation) {
        const char *testingScript = "def returnInt():\n"
                                    "   return 5\n";

        PyRun_SimpleString(testingScript);
        sweetPy::object_ptr result = sweetPy::Python::InvokeFunction("__main__", "", "returnInt");
        ASSERT_EQ(5, sweetPy::Object<int>::FromPython(result.get()));
    }

    TEST(CPythonClassTest, NonOveridedVirtualFunctionCall) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "a.IncBaseValue()\n"
                                    "result = a.GetBaseValue()";

        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("result"), 1);
    }


    TEST(CPythonClassTest, StaticMethod) {
        const char *testingScript = "TestClass.Setter()\n"
                                    "value = TestClass.Getter()";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(TestSubjectA::Getter(), true);
        ASSERT_EQ(TestSubjectA::Getter(), PythonEmbedder::GetAttribute<bool>("value"));
    }


    TEST(CPythonClassTest, PODByValueArgumentConstructor) {
        const char *testingScript = "a = TestClass(7)\n";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("a.byValueInt"), 7);
    }
    //Invoking a compiler generated constructor, no CPythonConstructor was binded to the python representation.
    TEST(CPythonClassTest, InvokingCompilerGeneratedContructor) {
        const char *testingScript = "a = TestClassB()";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("a.str"), "Hello World");
    }

    TEST(CPythonClassTest, PODByValueMember) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "a.byValueInt = 5";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("a.byValueInt"), 5);
    }

    TEST(CPythonClassTest, DestructorCall) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "del a";
        TestSubjectA::m_instanceDestroyed = false;
        ASSERT_EQ(TestSubjectA::m_instanceDestroyed, false);
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(TestSubjectA::m_instanceDestroyed, true);
    }

    TEST(CPythonClassTest, VirtualFunctionCall) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "a.byValueInt = 5\n"
                                    "b = a.GetValue()";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("a.byValueInt"), 5);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("a.byValueInt"), PythonEmbedder::GetAttribute<int>("b"));
    }

    TEST(CPythonClassTest, HandleArgumentConversionofNonPythonType) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "s = 'Hello World'\n"
                                    "newS = a.SetString(s)";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("s"), std::string("Hello World"));
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("newS"), std::string("Hello World Temp"));
    }
    //Test is going to verify the capability to return a reference of type T&.
    //The reference object is wrapped when being tranformed from the C++ to Python memory model.
    //in this test a.GetB() will return a reference wrapper object pointed by python's module entry b.
    //b is then provided back to a native function which will unpack the reference object.
    //at the end the undeline buffer is modified and the result is asserted.
    TEST(CPythonClassTest, LvalueReferenceArgumentReturnPassing) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "b = a.GetB()\n"
                                    "TestClass.BMutator(b)";
        PyRun_SimpleString(testingScript);
        const TestSubjectB& b = PythonEmbedder::GetAttribute<const TestSubjectB&>("b");
        ASSERT_EQ(b.GetValue(), 1);
    }
    //Test is going to address the capability of providing a wrapped reference of type T&, where T
    //is non copyable and non moveable. the returned wrapped will be invoked upon, changing the internal state
    //of the wrapped object. the result of that modification will be inspected.
    TEST(CPythonClassTest, LvalueReferenceArgumentReturnNonCopy_MoveAblePassing) {
        const char *testingScript = "a = TestClassC.instance()\n"
                                    "a.inc()";
        PyRun_SimpleString(testingScript);
        TestSubjectC& a = PythonEmbedder::GetAttribute<TestSubjectC&>("a");
        ASSERT_EQ(a.i, 1);
    }

    TEST(CPythonClassTest, NativeReturn) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "b = a.GetBByNoCopyConstructible()\n"
                                    "a.IncBByRef(b)\n"
                                    "b_2 = a.GetBByNoCopyConstructible()\n"
                                    "a.IncB(b_2)";
        PyRun_SimpleString(testingScript);
        std::unique_ptr<TestSubjectB>& b = PythonEmbedder::GetAttribute<std::unique_ptr<TestSubjectB>&>("b");
        ASSERT_EQ(b->GetValue(), 1);
        std::unique_ptr<TestSubjectB>& b_2 = PythonEmbedder::GetAttribute<std::unique_ptr<TestSubjectB>&>("b_2");
        ASSERT_EQ(b_2.get(), nullptr);
    }


    TEST(CPythonClassTest, InvocationUponReturnByValue){
        const char *testingScript = "a = TestClass(7)\n"
                                    "b = a.GetBByValue()\n"
                                    "b.IncValue()\n"
                                    "b_value = b.value";

        PyRun_SimpleString(testingScript);
        TestSubjectB& b = PythonEmbedder::GetAttribute<TestSubjectB&>("b");
        ASSERT_EQ(b.GetValue(), 1);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("b_value"), 1);
    }


    TEST(CPythonClassTest, AccessNonPythonTypes){
        const char *testingScript = "a = TestClass(7)\n"
                                    "b = a.GetBByValue()\n"
                                    "def get(a = str):\n"
                                    "   return a\n"
                                    "c = get(b.str)";

        PyRun_SimpleString(testingScript);
        ASSERT_EQ(strcmp(PythonEmbedder::GetAttribute<std::string>("c").c_str(), "Hello World") == 0, true);
    }

    TEST(CPythonClassTest, Enum) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "a.SetPython(Enum_Python.Bad)";
        PyRun_SimpleString(testingScript);
        TestSubjectA& a = PythonEmbedder::GetAttribute<TestSubjectA&>("a");
        ASSERT_EQ(a.m_enumValue, Python::Bad);
    }

    TEST(CPythonClassTest, InvocationViaReference) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "aRef = a.GetMe()\n"
                                    "s = 'Hello World'\n"
                                    "newS = aRef.SetString(s)";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("s"), std::string("Hello World"));
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("newS"), std::string("Hello World Temp"));
    }

    TEST(CPythonClassTest, OverloadingSupport) {
        const char *testingScript = "a = TestClassB()\n"
                                    "result1 = a.Foo_1(5)\n"
                                    "result2 = a.Foo_2(1, 3)\n"
                                    "result = result1 + result2";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("result"), 9);
    }

    TEST(CPythonClassTest, GlobalFunctionInvocation) {
        const char *testingScript = "result = globalFunction(5)";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("result"), 5);
    }

    TEST(CPythonClassTest, FromPythonStrToNativeXpireStrArgument) {
        const char *testingScript = "a = TestClass(5)\n"
                                    "a.SetXpireValue('Xpire Value')\n"
                                    "strRef = a.GetStr()\n";
        PyRun_SimpleString(testingScript);
        const std::string& strRef = PythonEmbedder::GetAttribute<const std::string&>("strRef");
        ASSERT_EQ(strRef, std::string("Xpire Value"));
    }

    TEST(CPythonClassTest, FromPythonListToNativeVector) {
        const char *testingScript = "a = TestClass(5)\n"
                "intVec = a.FromStrVectorToIntVector(['hello', 'world!'])\n";
        PyRun_SimpleString(testingScript);
        std::vector<int> intVector = PythonEmbedder::GetAttribute<std::vector<int>>("intVec");
        ASSERT_EQ(intVector.size(), 2);
        ASSERT_EQ(intVector[0], 5);
        ASSERT_EQ(intVector[1], 6);
    }

    TEST(CPythonClassTest, AccessingGlobalVariable)
    {
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("globalVariableLiteral"), "Hello World");
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("globalVariableInt_rvalue"), 5);
        ASSERT_EQ(PythonEmbedder::GetAttribute<int>("globalVariableInt_lvalue_const"), 6);
        ASSERT_EQ(PythonEmbedder::GetAttribute<std::string>("globalVariableStr_rvalue"), "Hello World");
        TestSubjectB& b_rvalue = PythonEmbedder::GetAttribute<TestSubjectB&>("globalVariableUserType_rvalue");
        ASSERT_EQ(b_rvalue.m_str, "Hello World");
        ASSERT_EQ(b_rvalue.m_value, 0);
        TestSubjectB& b_lvalue = PythonEmbedder::GetAttribute<TestSubjectB&>("globalVariableUserType_lvalue");
        ASSERT_EQ(b_lvalue.m_str, "Hello World");
        ASSERT_EQ(b_lvalue.m_value, 0);
    }
    
    TEST(CPythonClassTest, Serialize)
    {
        const char *testingScript = "int_val = 6\n"
                                    "double_val = 5.4324213123\n"
                                    "bool_val = True\n"
                                    "str_val = 'literal string seems to work'\n"
                                    "tuple_val = (7, 3.213123, True, 'also tuple works')\n";
        PyRun_SimpleString(testingScript);
        auto contextSend = sweetPy::SweetPickleFactory::Instance().CreateContext(sweetPy::SerializeType::FlatBuffers);
        auto sweetPickleSend = sweetPy::SweetPickleFactory::Instance().Create(sweetPy::SerializeType::FlatBuffers);
        sweetPickleSend->Write(*contextSend, 5);
        sweetPickleSend->Write(*contextSend, 7.5313123);
        sweetPickleSend->Write(*contextSend, true);
        sweetPickleSend->Write(*contextSend, "its working");
        sweetPickleSend->Write(*contextSend, std::string("yes it really works"));
        sweetPy::Tuple tuple;
        tuple.AddElement(5);
        tuple.AddElement(5.5);
        tuple.AddElement(true);
        tuple.AddElement("A");
        sweetPickleSend->Write(*contextSend, tuple);
        sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::GetAttribute<PyObject*>("int_val"),
                                                                    &sweetPy::Deleter::Borrow));
        sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::GetAttribute<PyObject*>("double_val"),
                                                                 &sweetPy::Deleter::Borrow));
        sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::GetAttribute<PyObject*>("bool_val"),
                                                                 &sweetPy::Deleter::Borrow));
        sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::GetAttribute<PyObject*>("str_val"),
                                                                 &sweetPy::Deleter::Borrow));
        sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::GetAttribute<PyObject*>("tuple_val"),
                                                                 &sweetPy::Deleter::Borrow));
    
        sweetPy::SerializeContext::String buffer = contextSend->Finish(false);
    
        auto contextReceive = sweetPy::SweetPickleFactory::Instance().CreateContext(sweetPy::SerializeType::FlatBuffers);
        auto sweetPickleReceive = sweetPy::SweetPickleFactory::Instance().Create(sweetPy::SerializeType::FlatBuffers);
        sweetPickleReceive->StartRead(*contextReceive, buffer);
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Int);
        int valInt = 0;
        sweetPickleReceive->Read(valInt);
        ASSERT_EQ(valInt, 5);
        
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Double);
        double valDouble = 0;
        sweetPickleReceive->Read(valDouble);
        ASSERT_EQ(valDouble, 7.5313123);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Bool);
        bool valBool = false;
        sweetPickleReceive->Read(valBool);
        ASSERT_EQ(valBool, true);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::String);
        std::string valStr;
        sweetPickleReceive->Read(valStr);
        ASSERT_EQ(valStr, "its working");
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::String);
        std::string valStr_2;
        sweetPickleReceive->Read(valStr_2);
        ASSERT_EQ(valStr_2, "yes it really works");
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Tuple);
        sweetPy::Tuple valTuple;
        sweetPickleReceive->Read(valTuple);
        ASSERT_EQ(valTuple, tuple);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Int);
        int valPyInt = 0;
        sweetPickleReceive->Read(valPyInt);
        ASSERT_EQ(valPyInt, 6);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Double);
        double valPyDouble = 0;
        sweetPickleReceive->Read(valPyDouble);
        ASSERT_EQ(valPyDouble, 5.4324213123);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Bool);
        bool valPyBool = false;
        sweetPickleReceive->Read(valPyBool);
        ASSERT_EQ(valPyBool, true);
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::String);
        std::string valPyStr;
        sweetPickleReceive->Read(valPyStr);
        ASSERT_EQ(valPyStr, "literal string seems to work");
    
        ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::Tuple);
        sweetPy::Tuple valPyTuple;
        sweetPickleReceive->Read(valPyTuple);
        ASSERT_EQ(valPyTuple, PythonEmbedder::GetAttribute<sweetPy::Tuple>("tuple_val"));
    }
}

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new sweetPyTest::CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

