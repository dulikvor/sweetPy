#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <Python.h>

namespace sweetPy{

    class CPythonModule;
    class ICPythonFunction;
    class ICPythonClass;

    class CPyModuleContainer
    {
    public:
        static CPyModuleContainer& Instance();
        void AddModule(const std::string& key, const std::shared_ptr<CPythonModule>& module);
        CPythonModule& GetModule(const std::string& key);
        void AddType(size_t key, PyTypeObject* const type);
        void AddMethod(int key, std::shared_ptr<ICPythonFunction>& method);
        void AddStaticMethod(int key, std::shared_ptr<ICPythonFunction>& staticMethod);
        ICPythonFunction& GetMethod(int key);
        ICPythonFunction& GetStaticMethod(int key);
        bool Exists(size_t key);
        PyTypeObject* const GetType(size_t key);
        template<typename T>
        static size_t TypeHash(){
            return typeid(T).hash_code();
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<CPythonModule>> m_modules;
        std::unordered_map<size_t, PyTypeObject* const> m_types;
        std::unordered_map<int, std::shared_ptr<ICPythonFunction>> m_methods;
        std::unordered_map<int, std::shared_ptr<ICPythonFunction>> m_staticMethods;
    };
}
