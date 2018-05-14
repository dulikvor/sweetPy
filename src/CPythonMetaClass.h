#pragma once

#include <vector>
#include <memory>
#include <Python.h>
#include <structmember.h>
#include <CPythonFunction.h>
#include <CPythonEnumValue.h>

namespace pycppconn {

    struct TypeState{
    public:
        TypeState(const std::string& name, const std::string& doc):
                Name(name), Doc(doc), PyType(nullptr){}
        ~TypeState(){
            delete[] PyType->tp_methods;
            delete[] PyType->tp_members;
        }
    public:
        std::string Name;
        std::string Doc;
        std::unique_ptr<PyTypeObject> PyType;
    };

    class CPythonMetaClass {
    public:
        enum Collectable {
            False = 0,
            True = 1
        };

        CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc, int extendedSize = 0);
        ~CPythonMetaClass();
        void InitType();
        void AddToModule();
        void AddMethod(const std::shared_ptr<ICPythonFunction>& method);
        void AddEnumValue(std::unique_ptr<CPythonEnumValue>&& enumValue);
        PyTypeObject& ToPython() const;
        PyObject* InitializeSubType(const std::string& name, const std::string& doc) const;
        static PyTypeObject &GetStaticMetaType();
        static void InitStaticType();
        static int IsCollectable(PyObject *obj);

    private:
        void InitMethods();
        void InitEnumValues();

    private:
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::vector<std::unique_ptr<CPythonEnumValue>> m_cPythonEnumVales;
        std::unique_ptr<TypeState> m_typeState;
        static PyTypeObject m_staticType;
        CPythonModule& m_module;
    };
}


