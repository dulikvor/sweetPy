#include <string>
#include "src/CPythonModule.h"
#include "src/CPythonClass.h"

using namespace pycppconn;

void foo(std::string s, int i){}

int main( int argc, const char *argv[] )
{
    CPythonClass<int> c;
    c.AddMethod("", "", &foo);

    return 0;
}
