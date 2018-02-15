#include <string>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"
#include <Python.h>

using namespace pycppconn;

class A{
public:
    void foo(std::string s, int i){}
public:
    int i;
};

INIT_MODULE(A_Module, "A-doc")
{
    CPythonClass<A> c(module, "name", "doc");
    c.AddMethod("foo", "foo-doc", &A::foo);
    c.AddMember("i", &A::i, "i-doc");
}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();
    initA_Module();
    Py_Finalize();

    return 0;
}
