#pragma once

#include <vector>
#include <memory>
#include <Python.h>
#include <structmember.h>
#include "CPythonType.h"

namespace pycppconn {

    class CPythonEnumValue;
    class ICPythonFunction;
    class CPythonModule;

    class CPythonMetaClassType : public CPythonType
    {
    public:
        enum Collectable {
            False = 0,
            True = 1
        };

        CPythonMetaClassType(const std::string& name, const std::string& doc, int extendedSize);

    private:
        static int IsCollectable(PyObject *obj);
    };

    class CPythonMetaClass {
    public:
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

    private:
        void InitMethods();
        void InitEnumValues();

    private:
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::vector<std::unique_ptr<CPythonEnumValue>> m_cPythonEnumValues;
        std::unique_ptr<CPythonType> m_type;
        static PyTypeObject m_staticType;
        CPythonModule& m_module;
    };
}


