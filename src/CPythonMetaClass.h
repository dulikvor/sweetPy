#pragma once

#include <vector>
#include <memory>
#include <Python.h>
#include <structmember.h>
#include <CPythonFunction.h>

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

        CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc);
        ~CPythonMetaClass();
        void InitType();
        void AddMethod(const std::shared_ptr<ICPythonFunction>& method);
        PyTypeObject& ToPython() const;


        static PyTypeObject &GetStaticMetaType();
        static void InitStaticType();
        static int IsCollectable(PyObject *obj);

    private:
        void InitMethods();

    private:
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::unique_ptr<TypeState> m_typeState;
        static PyTypeObject m_staticType;
        CPythonModule& m_module;
    };
}


