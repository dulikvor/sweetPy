#pragma once

#include <Python.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "Core/Lock.h"
#include "Core/Deleter.h"
#include "Core/PythonAssist.h"
#include "src/Detail/CPythonObject.h"

namespace sweetPyTest{

    class PythonEmbedder{
    public:
        static PythonEmbedder& instance()
        {
            static PythonEmbedder instance;
            return instance;
        }

        void initiate_interperter(const char* programName, int argc, char** argv){
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
        void terminate_interperter(){
            Py_Finalize();
            for(auto& var : m_argv.second)
                PyMem_RawFree(var);
            delete[] m_argv.first;
        }

        template<typename T>
        static T get_attribute(const char* name){
            {
                return sweetPy::Object<T>::from_python(sweetPy::Python::get_attribute(name).get());
            }
        }
    private:
        std::unique_ptr<wchar_t, std::function<void(wchar_t*)>> m_name;
        std::pair<wchar_t**, std::vector<wchar_t*>> m_argv;
    };
}