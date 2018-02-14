#include <string>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"
#include <Python.h>

using namespace pycppconn;

class A{
public:
    void foo(std::string s, int i){}

    int i;
};

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();

    CPythonClass<A> c("name", "doc");
    c.AddMethod("foo", "foo-doc", &A::foo);
    c.AddMember("i", &A::i, "i-doc");
    PyTypeObject& obj = c.ToPython();
    Py_Finalize();

    return 0;
}
