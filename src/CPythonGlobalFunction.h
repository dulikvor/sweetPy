#pragma once

#include <string>
#include <memory>
#include "CPythonModule.h"
#include "CPythonConcreteFunction.h"
#include "CPyModuleContainer.h"
#include "CPythonMetaClass.h"

namespace sweetPy{

    class CPythonGlobalFunction {
    public:

        template<typename X, typename std::enable_if<std::is_function<typename std::remove_pointer<X>::type>::value, bool>::type = true>
        CPythonGlobalFunction(CPythonModule &module, const std::string &name, const std::string &doc, X &&function)
            :m_module(module), m_name(name), m_doc(doc)
        {
            typedef CPythonCFunction<X> CPyFuncType;
            m_function.reset(new CPyFuncType(name, doc, function));
            CPyModuleContainer::Instance().AddGlobalFunction(CPythonFunction::GenerateFunctionId<CPyFuncType>(m_name), m_function);
        }

        ~CPythonGlobalFunction()
        {
            m_module.AddGlobalFunction(m_function);
        }

    private:
        std::shared_ptr<CPythonFunction> m_function;
        std::string m_name;
        std::string m_doc;
        CPythonModule &m_module;
    };

}

