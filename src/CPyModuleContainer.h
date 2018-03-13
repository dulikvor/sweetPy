#pragma once

#include <unordered_map>
#include <memory>
#include <string>

namespace pycppconn{

    class CPythonModule;
    class ICPythonFunction;
    class ICPythonClass;

    class CPyModuleContainer
    {
    public:
        static CPyModuleContainer& Instance();
        void AddModule(const std::string& key, const std::shared_ptr<CPythonModule>& module);
        CPythonModule& GetModule(const std::string& key);
        void AddMethod(int key, const std::shared_ptr<ICPythonFunction>& method);
        void AddStaticMethod(int key, const std::shared_ptr<ICPythonFunction>& staticMethod);
        ICPythonFunction& GetMethod(int key);
        ICPythonFunction& GetStaticMethod(int key);

    private:
        std::unordered_map<std::string, std::shared_ptr<CPythonModule>> m_modules;
        std::unordered_map<int, std::shared_ptr<ICPythonFunction>> m_methods;
        std::unordered_map<int, std::shared_ptr<ICPythonFunction>> m_staticMethods;
    };
}
