#pragma once

#include <string>
#include <Python.h>

namespace pycppconnTest{

    class PythonEmbeder{
    public:
        PythonEmbeder(const std::string& name, int argc, char** argv, const std::string& script)
        :m_script(script){
            Py_SetProgramName(const_cast<char*>(name.c_str()));
            Py_Initialize();
            PySys_SetArgv(argc, argv);
        }
        ~PythonEmbeder(){
            Py_Finalize();
        }

        void Run(){
            PyRun_SimpleString(m_script.c_str());
        }
    private:
        std::string m_script;
    };
}