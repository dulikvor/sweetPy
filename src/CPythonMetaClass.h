#pragma once

#include <Python.h>
#include <vector>
#include <memory>
#include <structmember.h>
#include "Core/Deleter.h"
#include "CPythonType.h"

namespace sweetPy {

    class CPythonFunction;
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
        static void Dealloc(PyObject *object);
    };

    class CPythonMetaClass
    {
    public:
        CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc, int extendedSize = 0);
        ~CPythonMetaClass();
        static PyTypeObject& GetStaticMetaType();
        PyTypeObject& ToPython() const;

        void InitType();
        void AddStaticMethod(const std::shared_ptr<CPythonFunction>& staticMethod);
        static void InitStaticType();

    private:
        struct NonCollectableMetaType : public PyTypeObject
        {
        public:
            NonCollectableMetaType();
        };

    private:
        void InitStaticMethods();

    private:
        std::vector<std::shared_ptr<CPythonFunction>> m_staticMethods;
        CPythonModule& m_module;
        object_ptr m_type;
        static NonCollectableMetaType m_staticType;
    };
}


