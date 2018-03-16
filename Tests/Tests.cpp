#include "gtest/gtest.h"
#include <Python.h>
#include "CPythonClassTestModule.h"


static int _argc;
static char** _argv;


bool CPythonClassTestSubject::m_valid = false;

class CPythonClassTest : public ::testing::Environment{
public:
    CPythonClassTest(){}
    void SetUp() override{
        Py_SetProgramName("CPythonClassTest");
        Py_Initialize();
        PySys_SetArgv(_argc, _argv);
        const char* testingScript = "from CPythonClassTestModule import TestClass\n";
        PyRun_SimpleString(testingScript);
    }
    void TearDown() override{
        Py_Finalize();
    }
};

int main(int argc, char **argv) {
    _argc = argc;
    _argv = argv;
    ::testing::AddGlobalTestEnvironment(new CPythonClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


namespace pycppconnTest {
    TEST(CPythonClassTest, StaticMethod) {
        const char* testingScript = "TestClass.Setter()\n";
        PyRun_SimpleString(testingScript);
        ASSERT_EQ(CPythonClassTestSubject::Getter(), true);
    }

    TEST(CPythonClassTest, PODByValueMember) {
        const char* testingScript = "a = TestClass(7)\n"
                                    "print a.byValueInt\n"
                                    "a.byValueInt = 5\n"
                                    "print(a.byValueInt)";
        PyRun_SimpleString(testingScript);
    }
}
