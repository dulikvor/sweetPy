#include "gtest/gtest.h"
#include "EmbededPython.h"


static int _argc;
static char** _argv;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    _argc = argc;
    _argv = argv;
    return RUN_ALL_TESTS();
}

namespace pycppconnTest {
    TEST(CPythonClassTest, StaticMethod) {
        const char* testingScript = "from CPythonClassTestModule import TestClass\n"
                                    "TestClass.Setter()\n"
                                    "print('Tammer hello')";
        PythonEmbeder embeder("CPythonClassTest_StaticMethod_Test", _argc, _argv, testingScript);
        embeder.Run();
    }
}
