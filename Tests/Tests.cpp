#include <type_traits>
#include "gtest/gtest.h"
#include <Python.h>
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
            PythonEmbedder::InitiateInterperter("CPythonClassTest", _argc, _argv);
            const char *testingScript = "from CPythonClassTestModule import TestClass, TestClassB, TestClassC, Enum_Python\n";
            PyRun_SimpleString(testingScript);
        }

        void TearDown() override {
            PythonEmbedder::TerminateInterperter();
        }
    };

    //Test is going to validate at compile time the correctness of different types use cases
    // being handled by ObjectWrapper and Object.
    TEST(CPythonClassTest, CPythonObjectWrapperTest){
        {
            //The use case of non copyable/moveable reference type
            static_assert(std::is_same<typename sweetPy::ObjectWrapper<TestSubjectC&,0>::Type, TestSubjectC>::value, "ObjectWrapper Type - validating a non copyable/moveable reference type assertion has failed.");
            static_assert(std::is_same<typename sweetPy::ObjectWrapper<TestSubjectC&,0>::FromPythonType, PyObject*>::value, "ObjectWrapper FromPython - validating a non copyable/moveable reference type assertion has failed.");
        }
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

}

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new sweetPyTest::CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

