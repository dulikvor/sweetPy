#include <string>
#include <iostream>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"
#include <Python.h>

using namespace pycppconn;

class A{
public:
    A(int&& i, const char* str):m_i(i), m_str(str){}
    void foo(int i, int y){
        int b = i + y + m_i;
        auto tempStr = const_cast<char*>(m_str);
        tempStr[0] = 'a';
        std::cout<<m_str<<b<<std::endl;
    }
public:
    int m_i;
    const char* m_str;
};

INIT_MODULE(example, "Example doc")
{
    CPythonClass<A> c(module, "A", "doc");
    c.AddConstructor<int&&,const char*>();
    c.AddMethod("foo", "foo-doc", &A::foo);
    c.AddMember("i", &A::m_i, "i-doc");
    c.AddMember("str", &A::m_str, "python string type member");
}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();
    initexample();
    Py_Finalize();

    return 0;
}
