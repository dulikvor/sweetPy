#include "gtest/gtest.h"
#include <Python.h>
#include "PythonEmbedder.h"
#include "CPythonClassTestModule.h"

static int _argc;
static char **_argv;

namespace pycppconnTest {

    class CPythonClassTest : public ::testing::Environment {
    public:
        CPythonClassTest() {}

        void SetUp() override {
            PythonEmbedder::InitiateInterperter("CPythonClassTest", _argc, _argv);
            const char *testingScript = "from CPythonClassTestModule import TestClass, Enum_Python\n";
            PyRun_SimpleString(testingScript);
        }

        void TearDown() override {
            PythonEmbedder::TerminateInterperter();
        }
    };


    TEST(CPythonClassTest, StaticMethod) {
        const char *testingScript = "TestClass.Setter()\n"
                                    "value = TestClass.Getter()";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(TestSubjectA::Getter(), true);
        ASSERT_EQ(TestSubjectA::Getter(), PythonEmbedder::GetAttribute<bool>("value"));
    }


    TEST(CPythonClassTest, PODByValueArgumentConstructor) {
        const char *testingScript = "a = TestClass(7)";
        PyRun_SimpleString(testingScript);

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

    TEST(CPythonClassTest, LvalueReferenceArgumentReturnPassing) {
        const char *testingScript = "a = TestClass(7)\n"
                                    "b = a.GetB()\n"
                                    "TestClass.BMutator(b)";
        PyRun_SimpleString(testingScript);
        const TestSubjectB& b = PythonEmbedder::GetAttribute<const TestSubjectB&>("b");
        ASSERT_EQ(b.GetValue(), 1);
    }
    TEST(CPythonClassTest, Enum) {
        const char *testingScript = "print 'enum value - {0}'.format(Enum_Python.Good)";
        PyRun_SimpleString(testingScript);
    }
}

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new pycppconnTest::CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

