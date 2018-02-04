#pragma once

#include <unordered_map>
#include <memory>
#include <string>

namespace pycppconn{

    class CPyModule;

    class CPyModuleContainer
    {
    public:
        static CPyModuleContainer& Instance();
        void AddModule(const std::string& key, const std::shared_ptr<CPyModule>& module);
        CPyModule& GetModule(const std::string& key);
    private:
        std::unordered_map<std::string, std::shared_ptr<CPyModule>> m_modules;

    };
}
