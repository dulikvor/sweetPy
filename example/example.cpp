#include <Python.h>
#include <string>
#include <iostream>
#include <functional>
#include "core/Logger.h"
#include "sweetPy.h"

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
    //Clazz<std::function<void(int)>> f(module, "F", "doc");
    //f.add_method<decltype(&std::function<void(int)>::operator())>("foo", "foo-doc", &std::function<void(int)>::operator());
    
    Clazz<A> c(module, "A", "doc");
    c.add_constructor<int&&,const char*>();
    c.add_method("foo", "foo-doc", &A::foo);
    c.add_member("i", &A::m_i, "i-doc");
    c.add_member("str", &A::m_str, "str-doc");
    c.add_static_method("boo", "boo-doc", &A::boo);
}

int main( int argc, const char *argv[] )
{
    core::Logger::Instance().Start(core::TraceSeverity::Info);
    char* programName = const_cast<char*>(argv[0]);
    wchar_t* decodedName = Py_DecodeLocale(programName, nullptr);
    Py_SetProgramName(decodedName);
    Py_Initialize();
    PyInit_example();
    Py_Finalize();
    PyMem_RawFree(decodedName);

    return 0;
}
