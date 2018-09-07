#pragma once

#include <Python.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "src/Core/Lock.h"
#include "src/Core/Deleter.h"
#include "src/Core/PythonAssist.h"
#include "CPythonObject.h"

namespace sweetPyTest{

    class PythonEmbedder{
    public:
        static PythonEmbedder& Instance()
        {
            static PythonEmbedder instance;
            return instance;
        }

        void InitiateInterperter(const char* programName, int argc, char** argv){
            m_name = std::unique_ptr<wchar_t, std::function<void(wchar_t*)>>(
                    Py_DecodeLocale("CPythonClassTest", nullptr), [](wchar_t* name){PyMem_RawFree(name);});
            m_argv = {new wchar_t*[argc], std::vector<wchar_t*>()};
            for(int index = 0; index < argc; index++)
            {
                m_argv.second.emplace_back(Py_DecodeLocale(argv[index], nullptr));
                m_argv.first[index] = m_argv.second.back();
            }
            Py_SetProgramName(m_name.get());
            Py_Initialize();
            PySys_SetArgv(argc, m_argv.first);
        }
        void TerminateInterperter(){
            Py_Finalize();
            for(auto& var : m_argv.second)
                PyMem_RawFree(var);
            delete[] m_argv.first;
        }

        template<typename T>
        static T GetAttribute(const char* name){
            {
                return sweetPy::Object<T>::FromPython(sweetPy::Python::GetAttribute(name).get());
            }
        }
    private:
        std::unique_ptr<wchar_t, std::function<void(wchar_t*)>> m_name;
        std::pair<wchar_t**, std::vector<wchar_t*>> m_argv;
    };
}