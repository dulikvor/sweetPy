#pragma once

#include <Python.h>
#include <vector>
#include <string>
#include <memory>
#include "Lock.h"
#include "Deleter.h"
#include "CPythonArgument.h"

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

        template<typename T>
        static T GetAttribute(const char* name){
            {
                std::vector<std::string> tokens = Split(name);
                pycppconn::GilLock lock;
                PyObject* context = PyImport_AddModule("__main__");
                for(const std::string& token : tokens){
                    std::unique_ptr<PyObject, pycppconn::Deleter::Func> attributeName(PyString_FromString(token.c_str()), &pycppconn::Deleter::Owner);
                    context = PyObject_GetAttr(context, attributeName.get());
                }
                return pycppconn::Argument<T>::FromPython(context);
            }
        }
    private:
        static std::vector<std::string> Split(const std::string& str){
            std::vector<std::string> result;
            std::size_t start = 0, end = 0;
            while((end = str.find('.', start)) != std::string::npos){
                result.emplace_back(str.substr(start, end - start));
                start = end + 1;
            }
            result.emplace_back(str.substr(start));
            return result;
        }
    };
}