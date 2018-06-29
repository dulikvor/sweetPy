#include <string>
#include <iostream>
#include <Python.h>
#include "src/InitModule.h"
#include "src/CPythonClass.h"

using namespace sweetPy;

class A{
public:
    A(int&& i, const char* str):m_i(i), m_str(str){}
    void foo(int i, int y){
        int b = i + y + m_i;
        auto tempStr = const_cast<char*>(m_str);
        tempStr[0] = 'a';
        std::cout<<m_str<<b<<std::endl;
    }
    static void boo(){
        std::cout<<"static method"<<std::endl;
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
    c.AddMember("str", &A::m_str, "str-doc");
    c.AddStaticMethod("boo", "boo-doc", &A::boo);
}

int main( int argc, const char *argv[] )
{
    Py_SetProgramName(const_cast<char*>(argv[0]));
    Py_Initialize();
    initexample();
    Py_Finalize();

    return 0;
}
