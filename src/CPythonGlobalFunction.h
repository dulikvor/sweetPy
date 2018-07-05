#pragma once

#include <string>
#include <memory>
#include "CPythonType.h"
#include "CPythonModule.h"
#include "CPythonFunction.h"
#include "CPyModuleContainer.h"
#include "CPythonMetaClass.h"

namespace sweetPy{

    class CPythonFunctionType : public CPythonType
    {
    public:
        CPythonFunctionType(const std::string& name, const std::string& doc, PyTypeObject* const type)
                :CPythonType(name, doc)
        {
            ob_type = type;
            ob_refcnt = 1;
            ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(PyObject);
            tp_flags = Py_TPFLAGS_HAVE_CLASS |
                       Py_TPFLAGS_HAVE_WEAKREFS;
            tp_doc = m_doc.c_str();
        }
    };

    class CPythonGlobalFunction {
    public:

        template<typename X, typename std::enable_if<std::is_function<typename std::remove_pointer<X>::type>::value, bool>::type = true>
        CPythonGlobalFunction(CPythonModule &module, const std::string &name, const std::string &doc, X &&function)
            :m_module(module), m_name(name), m_doc(doc)
        {
            typedef CPythonFunction<X> CPyFuncType;
            m_function.reset(new CPyFuncType(name, doc, function));
            CPyModuleContainer::Instance().AddGlobalFunction(GenerateFunctionId<CPyFuncType>(), m_function);
        }

        ~CPythonGlobalFunction()
        {
            m_metaClass.reset(new CPythonMetaClass<false>(m_module, std::string(m_name) + "_MetaClass",
                                                          std::string(m_doc) + "_MetaClass"));

            m_metaClass->SetCallableOperator(m_function->ToPython()->Function);
            m_metaClass->InitType();
            m_metaClass->InitializeEnumType(m_name, m_doc);
        }

    private:

        template<typename CPythonFunctionType>
        static int GenerateFunctionId() {
            return typeid(CPythonFunctionType).hash_code();
        }
    private:
        std::unique_ptr<CPythonMetaClass<false>> m_metaClass;
        std::shared_ptr<ICPythonFunction> m_function;
        std::string m_name;
        std::string m_doc;
        CPythonModule &m_module;
    };

}

