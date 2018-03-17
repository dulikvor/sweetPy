#pragma once

#include <Python.h>

namespace pycppconnTest{

    class PythonEmbedder{
    public:
        static void InitiateInterperter(const char* programName, int argc, char** argv){
            Py_SetProgramName("CPythonClassTest");
            Py_Initialize();
            PySys_SetArgv(argc, argv);
        }
        static void TerminateInterperter(){
            Py_Finalize();
        }
    };
}