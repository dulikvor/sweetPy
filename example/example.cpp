#include <string>
#include <iostream>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"
#include <Python.h>

using namespace pycppconn;

class A{
public:
    A(int i):m_i(i){}
    void foo(int i, int y){
        int b = i + y;
        std::cout<<b<<std::endl;
    }
public:
    int m_i;
};

INIT_MODULE(example, "Example doc")
{
    CPythonClass<A> c(module, "A", "doc");
    c.AddConstructor<int>();
    c.AddMethod("foo", "foo-doc", &A::foo);
    c.AddMember("i", &A::m_i, "i-doc");
}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();
    initexample();
    Py_Finalize();

    return 0;
}
