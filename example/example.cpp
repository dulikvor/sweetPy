#include <string>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"

using namespace pycppconn;

void foo(std::string s, int i){}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();

    CPythonClass<int> c;
    c.AddMethod("", "", &foo);
    CPythonFunction<void(*)(std::string, int)> pyFunc("", "", &foo);
    pyFunc.Wrapper(nullptr, nullptr);

    Py_Finalize();

    return 0;
}
