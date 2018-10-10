#include <Python.h>
#include <type_traits>
#include <iostream>
#include "gtest/gtest.h"
#include "core/Logger.h"
#include "PythonEmbedder.h"
#include "CPythonClassTestModule.h"

static int _argc;
static char **_argv;

namespace sweetPyTest {

    class CPythonClassTest : public ::testing::Environment {
    public:
        CPythonClassTest() {}

        void SetUp() override {
            core::Logger::Instance().Start(core::TraceSeverity::Info);
            PythonEmbedder::Instance().InitiateInterperter("CPythonClassTest", _argc, _argv);
            const char *testingScript = "from CPythonClassTestModule import TestClass, Enum_Python, TestClassB, TestClassC, globalFunction, globalVariableStr, globalVariableInt\n"
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

    TEST(CPythonClassTest, AccessingGlobalVariable) {
    std::string variable = PythonEmbedder::GetAttribute<std::string>("globalVariableStr");
    ASSERT_EQ(variable, std::string("Hello World"));
    ASSERT_EQ(PythonEmbedder::GetAttribute<int>("globalVariableInt"), 5);
    }

}

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new sweetPyTest::CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

