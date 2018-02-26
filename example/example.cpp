#include <string>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"
#include <Python.h>

using namespace pycppconn;

class A{
public:
    void foo(int i, int y){
        int b = i + y;
    }
public:
    int i;
};

INIT_MODULE(example, "Example doc")
{
    CPythonClass<A> c(module, "A", "doc");
    c.AddMethod("foo", "foo-doc", &A::foo);
    c.AddMember("i", &A::i, "i-doc");
}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();
    initexample();
    Py_Finalize();

    return 0;
}
