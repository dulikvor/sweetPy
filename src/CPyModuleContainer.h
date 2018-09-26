#pragma once

#include <Python.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <atomic>
#include "Core/Deleter.h"

namespace sweetPy{

    class CPythonModule;
    class CPythonFunction;
    class ICPythonClass;

    class CPyModuleContainer
    {
    private:
        typedef std::unordered_map<std::string, std::shared_ptr<CPythonModule>> Modules;

    public:
        static CPyModuleContainer& Instance();
        ~CPyModuleContainer();
        void RegisterContainer();
        void AddModule(const std::string& key, const std::shared_ptr<CPythonModule>& module);
        CPythonModule& GetModule(const std::string& key);
        const Modules& GetModules() const;
        void AddType(size_t key, object_ptr&& type);
        void AddMethod(int key, std::shared_ptr<CPythonFunction>& method);
        void AddStaticMethod(int key, std::shared_ptr<CPythonFunction>& staticMethod);
        void AddGlobalFunction(int key, std::shared_ptr<CPythonFunction>& function);
        CPythonFunction& GetMethod(int key);
        CPythonFunction& GetStaticMethod(int key);
        CPythonFunction& GetGlobalFunction(int key);
        bool Exists(size_t key);
        PyTypeObject* const GetType(size_t key);
        template<typename T>
        static size_t TypeHash(){
            return typeid(T).hash_code();
        }

    private:
        CPyModuleContainer();
        void Clear();
        static PyObject* atExit(PyObject*, PyObject*);

    private:
        std::atomic_bool m_isRegistered;
        Modules m_modules;
        PyMethodDef m_atExitDescriptor;
        std::unordered_map<size_t, object_ptr> m_types;
        std::unordered_map<int, std::shared_ptr<CPythonFunction>> m_methods;
        std::unordered_map<int, std::shared_ptr<CPythonFunction>> m_staticMethods;
        std::unordered_map<int, std::shared_ptr<CPythonFunction>> m_functions;
    };
}
