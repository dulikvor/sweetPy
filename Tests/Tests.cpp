#include "gtest/gtest.h"
#include "EmbededPython.h"
#include "CPythonClassTestModule.h"


static int _argc;
static char** _argv;


bool CPythonClassTestSubject::m_valid = false;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    _argc = argc;
    _argv = argv;
    return RUN_ALL_TESTS();
}

namespace pycppconnTest {
    TEST(CPythonClassTest, StaticMethod) {
        const char* testingScript = "from CPythonClassTestModule import TestClass\n"
                                    "TestClass.Setter()\n";
        PythonEmbeder embeder("CPythonClassTest_StaticMethod_Test", _argc, _argv, testingScript);
        embeder.Run();
        ASSERT_EQ(CPythonClassTestSubject::Getter(), true);
    }
}
