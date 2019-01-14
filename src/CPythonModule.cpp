#include "CPythonModule.h"
#include "Core/Assert.h"
#include "Core/PythonAssist.h"
#include "CPythonVariable.h"
#include "CPythonFunction.h"
#include "CPythonType.h"

using namespace std;

namespace sweetPy{

    CPythonModule::CPythonModule(const string &name, const string &doc):
            m_moduleDef{}, m_module(nullptr, &Deleter::Borrow), m_name(name), m_doc(doc)
    {}

    CPythonModule::~CPythonModule()
    {
        m_variables.clear();
        m_globalFunctions.clear();
        m_enums.clear();

        m_descriptors.clear();
        m_types.clear();
        m_module.release();
    }

    void CPythonModule::AddEnum(const std::string& name, object_ptr &&dictionary) {
        m_enums.emplace_back(name, std::move(dictionary));
    }

    void CPythonModule::AddType(CPythonType* type) {
        m_types.emplace_back(type);
    }
    
    void CPythonModule::EraseType(CPythonType* type)
    {
        m_types.remove(type);
    }

    void CPythonModule::AddVariable(std::unique_ptr<CPythonVariable>&& variable){
        m_variables.emplace_back(std::move(variable));
    }

    void CPythonModule::AddGlobalFunction(const std::shared_ptr<CPythonFunction> &function) {
        m_globalFunctions.emplace_back(function);
    }

    PyObject* CPythonModule::GetModule() const
    {
        return m_module.get();
    }

    void CPythonModule::Finalize()
    {
        m_moduleDef.m_base = PyModuleDef_HEAD_INIT;
        m_moduleDef.m_name = m_name.c_str();
        m_moduleDef.m_doc = m_doc.c_str();
        m_module.reset(PyModule_Create(&m_moduleDef));
        CPYTHON_VERIFY(m_module.get() != nullptr, "Module registration failed");
        InitGlobalFunctions();
        InitTypes();
        InitVariables();
        InitEnums();
    }

    void CPythonModule::InitEnums()
    {
        Py_XINCREF(Py_None);
        object_ptr pyNone(Py_None, &Deleter::Owner);
        if(m_enums.empty() == false)
        {
            for(auto& enumPair : m_enums) {
                object_ptr name(PyUnicode_FromString(enumPair.first.c_str()), &Deleter::Owner);
                CPYTHON_VERIFY(name != nullptr, "Was unable to import enum name into python");
                object_ptr enumClass = Python::InvokeFunction("enum", "Enum", "EnumMeta._create_", name.get(), enumPair.second.get());
                CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), enumPair.first.c_str(), enumClass.release()) == 0, "Type registration with module failed");
            }
        }
    }

    void CPythonModule::InitGlobalFunctions()
    {
        if(m_globalFunctions.empty() == false)
        {
            m_descriptors.reserve(m_globalFunctions.size());
            for (const auto &function : m_globalFunctions)
            {
                function->AllocateTypes(*this);
                auto descriptor = function->ToPython();
                object_ptr name(PyUnicode_FromString(descriptor->ml_name), &Deleter::Owner);
                object_ptr cFunction(PyCFunction_NewEx(descriptor.get(), name.get(), NULL), &Deleter::Owner);
                CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), descriptor->ml_name, cFunction.release()) == 0, "global function registration with module failed");
                m_descriptors.emplace_back(descriptor.release());
            }
        }
    }

    void CPythonModule::InitVariables()
    {
        if(m_variables.empty() == false)
        {
            for(auto& variable : m_variables)
                CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), variable->GetName().c_str(), variable->ToPython().release()) == 0, "Type registration with module failed");
        }
    }

    void CPythonModule::InitTypes()
    {
        if(m_types.empty() == false)
        {
            for(auto& type : m_types)
            {
                Py_XINCREF(type);
                CPYTHON_VERIFY(PyModule_AddObject((PyObject*)m_module.get(), type->GetName().c_str(), (PyObject*)type) == 0, "Type registration with module failed"); //Module added ref count ownership was taken by the module python's version
            }
        }
    }
}
