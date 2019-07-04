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
            PythonEmbedder::instance().initiate_interperter("CPythonClassTest", _argc, _argv);
            const char *testingScript = "from CPythonClassTestModule import TestClass, Enum_Python, TestClassB, TestClassC, globalFunction\n"
                                        "import CPythonClassTestModule as TestModule";
            PyRun_SimpleString(testingScript);
        }

        void TearDown() override {
            PythonEmbedder::instance().terminate_interperter();
        }
    };
    
    TEST(CPythonClassTest, FunctionInvocation) {
        const char *testingScript = "a = TestClass(7)\n"
                                    //Member functions - with return value and without.
                                    "a.IncBaseValue()\n"
                                    "result = a.GetBaseValue()\n"
                                    //Static member function invocation, with non specified user type as returned result
                                    "ptr = TestClass.GetUniqueMe()";
        
        //Member functions, with return value and without.
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(PythonEmbedder::get_attribute<int>("result"), 1);
        //Static member function invocation, returned type is non specified user type instance.
        auto& ptr = PythonEmbedder::get_attribute<std::unique_ptr<TestSubjectA>&>("ptr");
        ASSERT_EQ(ptr->m_byValueInt, 5);
    }
    
    /*
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
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<int>("intArgument"));
         ASSERT_EQ(1001, PythonEmbedder::get_attribute<int>("intReturn"));
         ASSERT_EQ(500, PythonEmbedder::get_attribute<int&>("intRefObject"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<int>("intReturn_2"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<const int&>("intConstRefObject"));
         ASSERT_EQ(502, PythonEmbedder::get_attribute<int>("intReturn_3"));
 
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<int>("intArgument_2"));
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<const int&>("intConstRefObject_2"));
         ASSERT_EQ(500, PythonEmbedder::get_attribute<int&>("intRefObject_2"));
         ASSERT_EQ(500, PythonEmbedder::get_attribute<const int&>("intConstRefObject_3"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<const int&>("intConstRefObject_4"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<const int&>("intConstRefObject_5"));
 
         ASSERT_EQ(502, PythonEmbedder::get_attribute<int&>("intRefObject_3"));
         ASSERT_EQ(502, PythonEmbedder::get_attribute<int&>("intRefObject_4"));
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
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<int>("intArgument"));
         ASSERT_EQ(1001, PythonEmbedder::get_attribute<double>("doubleReturn"));
         ASSERT_EQ(1000.5, PythonEmbedder::get_attribute<double>("doubleArgument"));
         ASSERT_EQ(1001.5, PythonEmbedder::get_attribute<double>("doubleReturn_2"));
         ASSERT_EQ(500, PythonEmbedder::get_attribute<int&>("intRefObject"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<double>("doubleReturn_3"));
         ASSERT_EQ(501, PythonEmbedder::get_attribute<const int&>("intConstRefObject"));
         ASSERT_EQ(502, PythonEmbedder::get_attribute<double>("doubleReturn_4"));
         ASSERT_EQ(500.5, PythonEmbedder::get_attribute<double&>("doubleRefObject"));
         ASSERT_EQ(501.5, PythonEmbedder::get_attribute<double>("doubleReturn_5"));
         ASSERT_EQ(501.5, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject"));
         ASSERT_EQ(502.5, PythonEmbedder::get_attribute<double>("doubleReturn_6"));
 
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<int>("intArgument_2"));
         ASSERT_EQ(1000, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject_2"));
         ASSERT_EQ(1000.5, PythonEmbedder::get_attribute<double>("doubleArgument_2"));
         ASSERT_EQ(1000.5, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject_3"));
         ASSERT_EQ(500.5, PythonEmbedder::get_attribute<double&>("doubleRefObject_2"));
         ASSERT_EQ(500.5, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject_4"));
         ASSERT_EQ(501.5, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject_5"));
         ASSERT_EQ(501.5, PythonEmbedder::get_attribute<const double&>("doubleConstRefObject_6"));
 
         ASSERT_EQ(503.35, PythonEmbedder::get_attribute<double&>("doubleRefObject_3"));
         ASSERT_EQ(503.35, PythonEmbedder::get_attribute<double&>("doubleRefObject_4"));
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
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string>("strArgument"));
         ASSERT_EQ("hello world", PythonEmbedder::get_attribute<std::string>("strReturn"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string>("bytesArgument"));
         ASSERT_EQ("hello world", PythonEmbedder::get_attribute<std::string>("strReturn_2"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string&>("strRefObject"));
         ASSERT_EQ("hello world", PythonEmbedder::get_attribute<std::string>("strReturn_3"));
         //std::string&
         ASSERT_EQ("hello to all", PythonEmbedder::get_attribute<std::string&>("strRefObject_2"));
         ASSERT_EQ("hello to all", PythonEmbedder::get_attribute<std::string>("strRefReturn"));
         //const std::string&
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string>("strArgument_2"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<const std::string&>("strConstRefReturn"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string>("bytesArgument_2"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<const std::string&>("strConstRefReturn_2"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<std::string&>("strRefObject_3"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<const std::string&>("strConstRefReturn_3"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<const std::string&>("strConstRefObject"));
         ASSERT_EQ("hello", PythonEmbedder::get_attribute<const std::string&>("strConstRefReturn_4"));
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
         ASSERT_EQ(strcmp("lello",PythonEmbedder::get_attribute<char*>("bytesArgument")), 0);
         ASSERT_EQ(strcmp("lelo",PythonEmbedder::get_attribute<char*>("ctypeStringRefObject")), 0);
     }
 
 
     TEST(CPythonClassTest, CPythonObjectCheckPyObjectIntegralType)
     {
     const char *testingScript = "pyObjectArgument = 5\n"
                                 "pyObjectReturn = TestModule.check_pyobject_conversion(pyObjectArgument) #PyObject* -> PyObject*\n";
 
     PyRun_SimpleString(testingScript);
     ASSERT_EQ(PythonEmbedder::get_attribute<PyObject*>("pyObjectReturn"), PythonEmbedder::get_attribute<PyObject*>("pyObjectArgument"));
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
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<sweetPy::DateTime>("datetimeArgument"));
         ASSERT_EQ(sweetPy::DateTime(1, 2, 3, 4, 5), PythonEmbedder::get_attribute<sweetPy::DateTime>("datetimeReturn"));
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::DateTime&>("datetimeConstRefObject"));
         ASSERT_EQ(sweetPy::DateTime(1, 2, 3, 4, 5), PythonEmbedder::get_attribute<sweetPy::DateTime>("datetimeReturn_2"));
         //const DateTime&
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<sweetPy::DateTime>("datetimeArgument_3"));
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::DateTime&>("datetimeReturn_3"));
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<sweetPy::DateTime>("datetimeArgument_4"));
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::DateTime&>("datetimeConstRefObject_2"));
         ASSERT_EQ(sweetPy::DateTime(9, 8, 7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::DateTime&>("datetimeReturn_4"));
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
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<sweetPy::TimeDelta>("timedeltaArgument"));
         ASSERT_EQ(sweetPy::TimeDelta(3, 4, 5), PythonEmbedder::get_attribute<sweetPy::TimeDelta>("timedeltaReturn"));
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::TimeDelta&>("timedeltaConstRefObject"));
         ASSERT_EQ(sweetPy::TimeDelta(3, 4, 5), PythonEmbedder::get_attribute<sweetPy::TimeDelta>("timedeltaReturn_2"));
         //const TimeDelta&
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<sweetPy::TimeDelta>("timedeltaArgument_3"));
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::TimeDelta&>("timedeltaReturn_3"));
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<sweetPy::TimeDelta>("timedeltaArgument_4"));
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::TimeDelta&>("timedeltaConstRefObject_2"));
         ASSERT_EQ(sweetPy::TimeDelta(7, 0, 6), PythonEmbedder::get_attribute<const sweetPy::TimeDelta&>("timedeltaReturn_4"));
     }
 
     TEST(CPythonClassTest, CPythonObjectCheckTupleType)
     {
 
         const char *testingScript = "from datetime import timedelta\n"
                 //Tuple
                 "tupleArgument = (1, 1.2, 'hello', True, [2, 1.3], ('world', False))\n"
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
         sweetPy::Tuple tupleArgument = PythonEmbedder::get_attribute<sweetPy::Tuple>("tupleReturn");
         ASSERT_EQ(1, tupleArgument.GetElement<int>(0));
         ASSERT_EQ(2.5, tupleArgument.GetElement<double>(1));
         ASSERT_EQ(std::string("Goodbye"), tupleArgument.GetElement<std::string>(2));
         ASSERT_EQ(std::string("World"), tupleArgument.GetElement<std::string>(3));
         ASSERT_EQ(true, tupleArgument.GetElement<bool>(4));
         sweetPy::List list_candidate_1;
         list_candidate_1.AddElement(2);
         list_candidate_1.AddElement(1.3);
         ASSERT_EQ(list_candidate_1, tupleArgument.GetElement<sweetPy::List>(4));
         sweetPy::Tuple tuple_candidate_1;
         tuple_candidate_1.AddElement("world");
         tuple_candidate_1.AddElement(false);
         ASSERT_EQ(tuple_candidate_1, tupleArgument.GetElement<sweetPy::Tuple>(5));
 
         const sweetPy::Tuple& tupleConstRefObject = PythonEmbedder::get_attribute<const sweetPy::Tuple&>("tupleConstRefObject");
         ASSERT_EQ(5, tupleConstRefObject.GetElement<int>(0));
         ASSERT_EQ(7.2, tupleConstRefObject.GetElement<double>(1));
 
         sweetPy::Tuple tupleReturn_2 = PythonEmbedder::get_attribute<sweetPy::Tuple>("tupleReturn_2");
         ASSERT_EQ(1, tupleReturn_2.GetElement<int>(0));
         ASSERT_EQ(2.5, tupleReturn_2.GetElement<double>(1));
         ASSERT_EQ(std::string("Goodbye"), tupleReturn_2.GetElement<std::string>(2));
         ASSERT_EQ(std::string("World"), tupleReturn_2.GetElement<std::string>(3));
         ASSERT_EQ(true, tupleReturn_2.GetElement<bool>(4));
         //const Tuple&
         sweetPy::Tuple tupleArgument_3 = PythonEmbedder::get_attribute<sweetPy::Tuple>("tupleArgument_3");
         ASSERT_EQ(0, tupleArgument_3.GetElement<bool>(0));
         ASSERT_EQ(std::string("to all"), tupleArgument_3.GetElement<std::string>(1));
 
         const sweetPy::Tuple& tupleReturn_3 = PythonEmbedder::get_attribute<const sweetPy::Tuple&>("tupleReturn_3");
         ASSERT_EQ(0, tupleReturn_3.GetElement<bool>(0));
         ASSERT_EQ(std::string("to all"), tupleReturn_3.GetElement<std::string>(1));
 
         sweetPy::Tuple tupleArgument_4 = PythonEmbedder::get_attribute<sweetPy::Tuple>("tupleArgument_4");
         ASSERT_EQ(nullptr, tupleArgument_4.GetElement<void*>(0));
 
         const sweetPy::Tuple& tupleConstRefObject_2 = PythonEmbedder::get_attribute<const sweetPy::Tuple&>("tupleConstRefObject_2");
         ASSERT_EQ(nullptr, tupleConstRefObject_2.GetElement<void*>(0));
 
         const sweetPy::Tuple& tupleReturn_4 = PythonEmbedder::get_attribute<const sweetPy::Tuple&>("tupleReturn_4");
         ASSERT_EQ(nullptr, tupleReturn_4.GetElement<void*>(0));
         //Transformation of non supported type element into python representation
         TestSubjectB& value = PythonEmbedder::get_attribute<TestSubjectB&>("value");
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
         //user type
         tuple.AddElement(TestSubjectA(5));
         ASSERT_EQ(5, tuple.GetElement<TestSubjectA>(0).GetValue());
         TestSubjectA user_type_val(6);
         tuple.AddElement(user_type_val);
         ASSERT_EQ(6, tuple.GetElement<TestSubjectA>(1).GetValue());
         const TestSubjectA const_user_type_val(7);
         tuple.AddElement(const_user_type_val);
         ASSERT_EQ(7, tuple.GetElement<TestSubjectA>(2).GetValue());
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
     
     TEST(CPythonClassTest, CPythonObjectCheckListType)
     {
         
         const char *testingScript = "from datetime import timedelta\n"
                                     //List
                                     "listArgument = [1, 1.2, 'hello', True]\n"
                                     "listReturn = TestModule.check_list_conversion(listArgument) #list -> List\n"
                                     "listConstRefGenerator = TestModule.GenerateListConstRef()\n"
                                     "listArgument_2 = [5, 7.2]\n"
                                     "listConstRefObject = listConstRefGenerator.create(listArgument_2)\n"
                                     "listReturn_2 = TestModule.check_list_conversion(listConstRefObject) #const List& -> List\n"
                                     //const List&
                                     "listArgument_3 = [False, 'to all']\n"
                                     "listReturn_3 = TestModule.check_const_ref_list_conversion(listArgument_3) #list -> const List&\n"
                                     "listArgument_4 = [None,]\n"
                                     "listConstRefObject_2 = listConstRefGenerator.create(listArgument_4)\n"
                                     "listReturn_4 = TestModule.check_const_ref_list_conversion(listConstRefObject_2) #const List& -> const List&\n"
                                     //Transformation of non supported type element into python representation
                                     "listReturn_5 = TestModule.generate_native_element_list()\n"
                                     "value = listReturn_5[0]";
         
         PyRun_SimpleString(testingScript);
         //List
         sweetPy::List listArgument = PythonEmbedder::get_attribute<sweetPy::List>("listReturn");
         ASSERT_EQ(1, listArgument.GetElement<int>(0));
         ASSERT_EQ(2.5, listArgument.GetElement<double>(1));
         ASSERT_EQ(std::string("Goodbye"), listArgument.GetElement<std::string>(2));
         ASSERT_EQ(std::string("World"), listArgument.GetElement<std::string>(3));
         ASSERT_EQ(true, listArgument.GetElement<bool>(4));
         
         const sweetPy::List& listConstRefObject = PythonEmbedder::get_attribute<const sweetPy::List&>("listConstRefObject");
         ASSERT_EQ(5, listConstRefObject.GetElement<int>(0));
         ASSERT_EQ(7.2, listConstRefObject.GetElement<double>(1));
         
         sweetPy::List listReturn_2 = PythonEmbedder::get_attribute<sweetPy::List>("listReturn_2");
         ASSERT_EQ(1, listReturn_2.GetElement<int>(0));
         ASSERT_EQ(2.5, listReturn_2.GetElement<double>(1));
         ASSERT_EQ(std::string("Goodbye"), listReturn_2.GetElement<std::string>(2));
         ASSERT_EQ(std::string("World"), listReturn_2.GetElement<std::string>(3));
         ASSERT_EQ(true, listReturn_2.GetElement<bool>(4));
         //const List&
         sweetPy::List listArgument_3 = PythonEmbedder::get_attribute<sweetPy::List>("listArgument_3");
         ASSERT_EQ(0, listArgument_3.GetElement<bool>(0));
         ASSERT_EQ(std::string("to all"), listArgument_3.GetElement<std::string>(1));
         
         const sweetPy::List& listReturn_3 = PythonEmbedder::get_attribute<const sweetPy::List&>("listReturn_3");
         ASSERT_EQ(0, listReturn_3.GetElement<bool>(0));
         ASSERT_EQ(std::string("to all"), listReturn_3.GetElement<std::string>(1));
         
         sweetPy::List listArgument_4 = PythonEmbedder::get_attribute<sweetPy::List>("listArgument_4");
         ASSERT_EQ(nullptr, listArgument_4.GetElement<void*>(0));
         
         const sweetPy::List& listConstRefObject_2 = PythonEmbedder::get_attribute<const sweetPy::List&>("listConstRefObject_2");
         ASSERT_EQ(nullptr, listConstRefObject_2.GetElement<void*>(0));
         
         const sweetPy::List& listReturn_4 = PythonEmbedder::get_attribute<const sweetPy::List&>("listReturn_4");
         ASSERT_EQ(nullptr, listReturn_4.GetElement<void*>(0));
         //Transformation of non supported type element into python representation
         TestSubjectB& value = PythonEmbedder::get_attribute<TestSubjectB&>("value");
         ASSERT_EQ(value.GetValue(), 0);
         ASSERT_EQ(value.GetStr(), "Hello World");
         
         //Check native type
         sweetPy::List list;
         //int
         list.AddElement(5);
         ASSERT_EQ(5, list.GetElement<int>(0));
         int int_val = 6;
         list.AddElement(int_val);
         ASSERT_EQ(6, list.GetElement<int>(1));
         const int const_int_val = 7;
         list.AddElement(const_int_val);
         ASSERT_EQ(7, list.GetElement<int>(2));
         list.Clear();
         //double
         list.AddElement(5.5);
         ASSERT_EQ(5.5, list.GetElement<double>(0));
         double double_val = 6.5;
         list.AddElement(double_val);
         ASSERT_EQ(6.5, list.GetElement<double>(1));
         const double const_double_val = 7.5;
         list.AddElement(const_double_val);
         ASSERT_EQ(7.5, list.GetElement<double>(2));
         list.Clear();
         //bool
         list.AddElement(true);
         ASSERT_EQ(true, list.GetElement<bool>(0));
         bool bool_val = false;
         list.AddElement(bool_val);
         ASSERT_EQ(0, list.GetElement<bool>(1));
         const bool const_bool_val = false;
         list.AddElement(const_bool_val);
         ASSERT_EQ(0, list.GetElement<bool>(2));
         list.Clear();
         //Ctype string
         list.AddElement("A");
         ASSERT_EQ("A", list.GetElement<std::string>(0));
         const char* cstr_val = "B";
         list.AddElement(const_cast<char*>(cstr_val));
         ASSERT_EQ("B", list.GetElement<std::string>(1));
         const char* const_cstr_val = "C";
         list.AddElement(const_cstr_val);
         ASSERT_EQ("C", list.GetElement<std::string>(2));
         list.Clear();
         //String
         list.AddElement(std::string("A"));
         ASSERT_EQ(std::string("A"), list.GetElement<std::string>(0));
         std::string str_val = "B";
         list.AddElement(str_val);
         ASSERT_EQ(std::string("B"), list.GetElement<std::string>(1));
         const std::string const_str_val = "C";
         list.AddElement(const_str_val);
         ASSERT_EQ(std::string("C"), list.GetElement<std::string>(2));
         list.Clear();
         //void*
         list.AddElement(nullptr);
         ASSERT_EQ(nullptr, list.GetElement<void*>(0));
         void* ptr_val = nullptr;
         list.AddElement(ptr_val);
         ASSERT_EQ(nullptr, list.GetElement<void*>(1));
         const void* const_ptr_val = nullptr;
         list.AddElement(const_ptr_val);
         ASSERT_EQ(nullptr, list.GetElement<void*>(2));
         list.Clear();
         //Copy constructor
         list.AddElement(5);
         list.AddElement(5.5);
         list.AddElement(true);
         list.AddElement("A");
         list.AddElement(std::string("B"));
         list.AddElement(nullptr);
         sweetPy::List cpy_list(list);
         ASSERT_EQ(cpy_list.GetElement<int>(0), list.GetElement<int>(0));
         ASSERT_EQ(cpy_list.GetElement<double>(1), list.GetElement<double>(1));
         ASSERT_EQ(cpy_list.GetElement<bool>(2), list.GetElement<bool>(2));
         ASSERT_EQ(cpy_list.GetElement<std::string>(3), list.GetElement<std::string>(3));
         ASSERT_EQ(cpy_list.GetElement<std::string>(4), list.GetElement<std::string>(4));
         ASSERT_EQ(cpy_list.GetElement<void*>(5), list.GetElement<void*>(5));
         list.Clear();
         //Copy assigment
         list.AddElement(5);
         list.AddElement(5.5);
         list.AddElement(true);
         list.AddElement("A");
         list.AddElement(std::string("B"));
         list.AddElement(nullptr);
         sweetPy::List cpy_assign_list;
         cpy_assign_list = list;
         ASSERT_EQ(cpy_assign_list.GetElement<int>(0), list.GetElement<int>(0));
         ASSERT_EQ(cpy_assign_list.GetElement<double>(1), list.GetElement<double>(1));
         ASSERT_EQ(cpy_assign_list.GetElement<bool>(2), list.GetElement<bool>(2));
         ASSERT_EQ(cpy_assign_list.GetElement<std::string>(3), list.GetElement<std::string>(3));
         ASSERT_EQ(cpy_assign_list.GetElement<std::string>(4), list.GetElement<std::string>(4));
         ASSERT_EQ(cpy_assign_list.GetElement<void*>(5), list.GetElement<void*>(5));
         list.Clear();
         //Move constructor
         list.AddElement(5);
         list.AddElement(5.5);
         list.AddElement(true);
         list.AddElement("A");
         list.AddElement(std::string("B"));
         list.AddElement(nullptr);
         sweetPy::List mv_list(std::move(list));
         ASSERT_EQ(mv_list.GetElement<int>(0), 5);
         ASSERT_EQ(mv_list.GetElement<double>(1), 5.5);
         ASSERT_EQ(mv_list.GetElement<bool>(2), true);
         ASSERT_EQ(mv_list.GetElement<std::string>(3), std::string("A"));
         ASSERT_EQ(mv_list.GetElement<std::string>(4), std::string("B"));
         ASSERT_EQ(mv_list.GetElement<void*>(5), nullptr);
         list.Clear();
         //Move assigment
         list.AddElement(5);
         list.AddElement(5.5);
         list.AddElement(true);
         list.AddElement("A");
         list.AddElement(std::string("B"));
         list.AddElement(nullptr);
         sweetPy::List mv_assign_list;
         mv_assign_list = std::move(list);
         ASSERT_EQ(mv_assign_list.GetElement<int>(0), 5);
         ASSERT_EQ(mv_assign_list.GetElement<double>(1), 5.5);
         ASSERT_EQ(mv_assign_list.GetElement<bool>(2), true);
         ASSERT_EQ(mv_assign_list.GetElement<std::string>(3), std::string("A"));
         ASSERT_EQ(mv_assign_list.GetElement<std::string>(4), std::string("B"));
         ASSERT_EQ(mv_assign_list.GetElement<void*>(5), nullptr);
         list.Clear();
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
         ASSERT_EQ(std::string("Babylon 5 Rulezzzzz!"), static_cast<const std::string&>(PythonEmbedder::get_attribute<sweetPy::AsciiString>("asciistrReturn")));
         ASSERT_EQ(std::string("Justice to all"), static_cast<const std::string&>(PythonEmbedder::get_attribute<const sweetPy::AsciiString&>("asciistrConstRefObject")));
         ASSERT_EQ(std::string("Babylon 5 Rulezzzzz!"), static_cast<const std::string&>(PythonEmbedder::get_attribute<sweetPy::AsciiString>("asciistrReturn_2")));
         //const TimeDelta&
         ASSERT_EQ(std::string("Death magnetic is not bad"), static_cast<const std::string&>(PythonEmbedder::get_attribute<const sweetPy::AsciiString&>("asciistrReturn_3")));
         ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<const std::string&>(PythonEmbedder::get_attribute<const sweetPy::AsciiString&>("asciistrConstRefObject_2")));
         ASSERT_EQ(std::string("Lulu? come on?!"), static_cast<const std::string&>(PythonEmbedder::get_attribute<const sweetPy::AsciiString&>("asciistrReturn_4")));
         
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
         ASSERT_EQ("Make love", PythonEmbedder::get_attribute<std::string>("constCTypeStrReturn"));
         ASSERT_EQ("Fever is no joke", PythonEmbedder::get_attribute<std::string>("constCTypeStrReturn_3"));
         ASSERT_EQ("Long live Sheridan", PythonEmbedder::get_attribute<std::string>("constCTypeStrReturn_4"));
         //const char*&
         ASSERT_EQ(std::string(PythonEmbedder::get_attribute<const char*>("constCTypeStrReturn_5")), "Make love");
         ASSERT_EQ(std::string(PythonEmbedder::get_attribute<const char*>("constCTypeStrReturn_6")), "Long live Sheridan");
         ASSERT_EQ(std::string(PythonEmbedder::get_attribute<const char*>("constCTypeStrReturn_7")), "Fever is no joke");
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
         ASSERT_EQ(6, PythonEmbedder::get_attribute<int>("objectptrReturn"));
         //const object_ptr&
         ASSERT_EQ(4, sweetPy::Object<int>::FromPython(static_cast<const sweetPy::object_ptr&>(PythonEmbedder::get_attribute<const sweetPy::object_ptr&>("objectptrReturn_2")).get()));
         ASSERT_EQ(5, sweetPy::Object<int>::FromPython(static_cast<const sweetPy::object_ptr&>(PythonEmbedder::get_attribute<const sweetPy::object_ptr&>("objectptrReturn_3")).get()));
         auto& generator = PythonEmbedder::get_attribute<GenerateRefTypes<const sweetPy::object_ptr>&>("objectptrConstRefGenerator");
         generator.Clear();
     }
 
     TEST(CPythonClassTest, PythonFunctionInvocation) {
         const char *testingScript = "def returnInt():\n"
                                     "   return 5\n";
 
         PyRun_SimpleString(testingScript);
         sweetPy::object_ptr result = sweetPy::Python::InvokeFunction("__main__", "", "returnInt");
         ASSERT_EQ(5, sweetPy::Object<int>::FromPython(result.get()));
     }
 
     TEST(CPythonClassTest, StaticMethod) {
         const char *testingScript = "TestClass.Setter()\n"
                                     "value = TestClass.Getter()";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(TestSubjectA::Getter(), true);
         ASSERT_EQ(TestSubjectA::Getter(), PythonEmbedder::get_attribute<bool>("value"));
     }
 
 
     TEST(CPythonClassTest, PODByValueArgumentConstructor) {
         const char *testingScript = "a = TestClass(7)\n";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("a.byValueInt"), 7);
     }
     //Invoking a compiler generated constructor, no CPythonConstructor was binded to the python representation.
     TEST(CPythonClassTest, InvokingCompilerGeneratedContructor) {
         const char *testingScript = "a = TestClassB()";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("a.str"), "Hello World");
     }
 
     TEST(CPythonClassTest, PODByValueMember) {
         const char *testingScript = "a = TestClass(7)\n"
                                     "a.byValueInt = 5";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("a.byValueInt"), 5);
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
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("a.byValueInt"), 5);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("a.byValueInt"), PythonEmbedder::get_attribute<int>("b"));
     }
 
     TEST(CPythonClassTest, HandleArgumentConversionofNonPythonType) {
         const char *testingScript = "a = TestClass(7)\n"
                                     "s = 'Hello World'\n"
                                     "newS = a.SetString(s)";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("s"), std::string("Hello World"));
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("newS"), std::string("Hello World Temp"));
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
         const TestSubjectB& b = PythonEmbedder::get_attribute<const TestSubjectB&>("b");
         ASSERT_EQ(b.GetValue(), 1);
     }
     //Test is going to address the capability of providing a wrapped reference of type T&, where T
     //is non copyable and non moveable. the returned wrapped will be invoked upon, changing the internal state
     //of the wrapped object. the result of that modification will be inspected.
     TEST(CPythonClassTest, LvalueReferenceArgumentReturnNonCopy_MoveAblePassing) {
         const char *testingScript = "a = TestClassC.instance()\n"
                                     "a.inc()";
         PyRun_SimpleString(testingScript);
         TestSubjectC& a = PythonEmbedder::get_attribute<TestSubjectC&>("a");
         ASSERT_EQ(a.i, 1);
     }
 
     TEST(CPythonClassTest, NativeReturn) {
         const char *testingScript = "a = TestClass(7)\n"
                                     "b = a.GetBByNoCopyConstructible()\n"
                                     "a.IncBByRef(b)\n"
                                     "b_2 = a.GetBByNoCopyConstructible()\n"
                                     "a.IncB(b_2)";
         PyRun_SimpleString(testingScript);
         std::unique_ptr<TestSubjectB>& b = PythonEmbedder::get_attribute<std::unique_ptr<TestSubjectB>&>("b");
         ASSERT_EQ(b->GetValue(), 1);
         std::unique_ptr<TestSubjectB>& b_2 = PythonEmbedder::get_attribute<std::unique_ptr<TestSubjectB>&>("b_2");
         ASSERT_EQ(b_2.get(), nullptr);
     }
 
 
     TEST(CPythonClassTest, InvocationUponReturnByValue){
         const char *testingScript = "a = TestClass(7)\n"
                                     "b = a.GetBByValue()\n"
                                     "b.IncValue()\n"
                                     "b_value = b.value";
 
         PyRun_SimpleString(testingScript);
         TestSubjectB& b = PythonEmbedder::get_attribute<TestSubjectB&>("b");
         ASSERT_EQ(b.GetValue(), 1);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("b_value"), 1);
     }
 
 
     TEST(CPythonClassTest, AccessNonPythonTypes){
         const char *testingScript = "a = TestClass(7)\n"
                                     "b = a.GetBByValue()\n"
                                     "def get(a = str):\n"
                                     "   return a\n"
                                     "c = get(b.str)";
 
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(strcmp(PythonEmbedder::get_attribute<std::string>("c").c_str(), "Hello World") == 0, true);
     }
 
     TEST(CPythonClassTest, Enum) {
         const char *testingScript = "a = TestClass(7)\n"
                                     "a.SetPython(Enum_Python.Bad)";
         PyRun_SimpleString(testingScript);
         TestSubjectA& a = PythonEmbedder::get_attribute<TestSubjectA&>("a");
         ASSERT_EQ(a.m_enumValue, Python::Bad);
     }
 
     TEST(CPythonClassTest, InvocationViaReference) {
         const char *testingScript = "a = TestClass(7)\n"
                                     "aRef = a.GetMe()\n"
                                     "s = 'Hello World'\n"
                                     "newS = aRef.SetString(s)";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("s"), std::string("Hello World"));
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("newS"), std::string("Hello World Temp"));
     }
 
     TEST(CPythonClassTest, OverloadingSupport) {
         const char *testingScript = "a = TestClassB()\n"
                                     "result1 = a.Foo_1(5)\n"
                                     "result2 = a.Foo_2(1, 3)\n"
                                     "result = result1 + result2";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("result"), 9);
     }
 
     TEST(CPythonClassTest, GlobalFunctionInvocation) {
         const char *testingScript = "result = globalFunction(5)";
         PyRun_SimpleString(testingScript);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("result"), 5);
     }
 
     TEST(CPythonClassTest, FromPythonStrToNativeXpireStrArgument) {
         const char *testingScript = "a = TestClass(5)\n"
                                     "a.SetXpireValue('Xpire Value')\n"
                                     "strRef = a.GetStr()\n";
         PyRun_SimpleString(testingScript);
         const std::string& strRef = PythonEmbedder::get_attribute<const std::string&>("strRef");
         ASSERT_EQ(strRef, std::string("Xpire Value"));
     }
 
     TEST(CPythonClassTest, FromPythonListToNativeVector) {
         const char *testingScript = "a = TestClass(5)\n"
                 "intVec = a.FromStrVectorToIntVector(['hello', 'world!'])\n";
         PyRun_SimpleString(testingScript);
         std::vector<int> intVector = PythonEmbedder::get_attribute<std::vector<int>>("intVec");
         ASSERT_EQ(intVector.size(), 2);
         ASSERT_EQ(intVector[0], 5);
         ASSERT_EQ(intVector[1], 6);
     }
 
     TEST(CPythonClassTest, AccessingGlobalVariable)
     {
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("globalVariableLiteral"), "Hello World");
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("globalVariableInt_rvalue"), 5);
         ASSERT_EQ(PythonEmbedder::get_attribute<int>("globalVariableInt_lvalue_const"), 6);
         ASSERT_EQ(PythonEmbedder::get_attribute<std::string>("globalVariableStr_rvalue"), "Hello World");
         TestSubjectB& b_rvalue = PythonEmbedder::get_attribute<TestSubjectB&>("globalVariableUserType_rvalue");
         ASSERT_EQ(b_rvalue.m_str, "Hello World");
         ASSERT_EQ(b_rvalue.m_value, 0);
         TestSubjectB& b_lvalue = PythonEmbedder::get_attribute<TestSubjectB&>("globalVariableUserType_lvalue");
         ASSERT_EQ(b_lvalue.m_str, "Hello World");
         ASSERT_EQ(b_lvalue.m_value, 0);
     }
     
     TEST(CPythonClassTest, Serialize)
     {
         const char *testingScript = "int_val = 6\n"
                                     "double_val = 5.4324213123\n"
                                     "bool_val = True\n"
                                     "str_val = 'literal string seems to work'\n"
                                     "tuple_val = (7, 3.213123, True, 'also tuple works')\n"
                                     "list_val = [7, 3.213123, True, 'also tuple works']\n";
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
         sweetPy::List list;
         list.AddElement(5);
         list.AddElement(5.5);
         list.AddElement(true);
         list.AddElement("A");
         sweetPickleSend->Write(*contextSend, list);
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("int_val"),
                                                                     &sweetPy::Deleter::Borrow));
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("double_val"),
                                                                  &sweetPy::Deleter::Borrow));
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("bool_val"),
                                                                  &sweetPy::Deleter::Borrow));
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("str_val"),
                                                                  &sweetPy::Deleter::Borrow));
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("tuple_val"),
                                                                  &sweetPy::Deleter::Borrow));
         sweetPickleSend->Write(*contextSend, sweetPy::object_ptr(PythonEmbedder::get_attribute<PyObject*>("list_val"),
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
     
         ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::List);
         sweetPy::List valList;
         sweetPickleReceive->Read(valList);
         ASSERT_EQ(valList, list);
     
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
         ASSERT_EQ(valPyTuple, PythonEmbedder::get_attribute<sweetPy::Tuple>("tuple_val"));
     
         ASSERT_EQ(sweetPickleReceive->GetType(), sweetPy::serialize::all_types::List);
         sweetPy::List valPyList;
         sweetPickleReceive->Read(valPyList);
         ASSERT_EQ(valPyList, PythonEmbedder::get_attribute<sweetPy::List>("list_val"));
     }
     */
}

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new sweetPyTest::CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

