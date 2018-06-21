#include "CPythonMetaClass.h"
#include "CPythonModule.h"
#include "ICPythonFunction.h"
#include "CPythonEnumValue.h"

namespace pycppconn {

    CPythonMetaClassType::CPythonMetaClassType(const std::string &name, const std::string &doc,
                                               int extendedSize) : CPythonType(name, doc)
    {
        ob_type = &PyType_Type;
        ob_refcnt = 1;
        ob_size = 0;
        tp_name = m_name.c_str();
        tp_basicsize = (Py_ssize_t)(sizeof(PyHeapTypeObject) + extendedSize);
        tp_flags = Py_TPFLAGS_HAVE_CLASS |
                   Py_TPFLAGS_HAVE_WEAKREFS;
        tp_doc = m_doc.c_str();
        tp_is_gc = &IsCollectable;
        tp_base = &PyType_Type;
    }

    int CPythonMetaClassType::IsCollectable(PyObject *obj) {
        return Collectable::False;
    }

    static int IsCollectable(PyObject *obj) {
        return CPythonMetaClassType::Collectable::False;
    }

    PyTypeObject CPythonMetaClass::m_staticType = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "CPythonMeta",             /* tp_name */
            sizeof(PyObject),          /* tp_basicsize */
            0,                         /* tp_itemsize */
            NULL,                      /* tp_dealloc */
            0,                         /* tp_print */
            0,                         /* tp_getattr */
            0,                         /* tp_setattr */
            0,                         /* tp_compare */
            0,                         /* tp_repr */
            0,                         /* tp_as_number */
            0,                         /* tp_as_sequence */
            0,                         /* tp_as_mapping */
            0,                         /* tp_hash */
            0,                         /* tp_call */
            0,                         /* tp_str */
            0,                         /* tp_getattro */
            0,                         /* tp_setattro */
            0,                         /* tp_as_buffer */
            Py_TPFLAGS_HAVE_CLASS |
            Py_TPFLAGS_HAVE_WEAKREFS,  /* tp_flags */
            "CPthon meta class",       /* tp_doc */
            0,                         /* tp_traverse */
            0,                         /* tp_clear */
            0,                         /* tp_richcompare */
            0,                         /* tp_weaklistoffset */
            0,                         /* tp_iter */
            0,                         /* tp_iternext */
            NULL,                      /* tp_methods */
            NULL,                      /* tp_members */
            0,                         /* tp_getset */
            &PyType_Type,              /* tp_base */
            0,                         /* tp_dict */
            0,                         /* tp_descr_get */
            0,                         /* tp_descr_set */
            0,                         /* tp_dictoffset */
            NULL,                      /* tp_init */
            0,                         /* tp_alloc */
            NULL,                      /* tp_new */
            NULL,                      /* Low-level free-memory routine */
            &IsCollectable, /* For PyObject_IS_GC */
    };

    CPythonMetaClass::CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc, int extendedSize)
            :m_module(module), m_type(new CPythonMetaClassType(name, doc, extendedSize)){
        Py_IncRef((PyObject*)m_type.get());
    }

    CPythonMetaClass::~CPythonMetaClass(){}

    PyTypeObject& CPythonMetaClass::GetStaticMetaType() {
        return m_staticType; }

    PyTypeObject& CPythonMetaClass::ToPython() const{
        return *m_type;
    }

    void CPythonMetaClass::InitType(){
        InitMethods();
        InitEnumValues();
        PyType_Ready(m_type.get());
    }

    void CPythonMetaClass::AddToModule()
    {
        m_module.AddType(std::move(m_type));
    }


    void CPythonMetaClass::AddMethod(const std::shared_ptr<ICPythonFunction>& method){
        m_cPythonMemberFunctions.emplace_back(method);
    }

    void CPythonMetaClass::AddEnumValue(std::unique_ptr<CPythonEnumValue>&& enumValue)
    {
        m_cPythonEnumValues.emplace_back(std::move(enumValue));
    }

    void CPythonMetaClass::InitMethods(){
        PyMethodDef* methods = new PyMethodDef[m_cPythonMemberFunctions.size() + 1]; //spare space for sentinal
        m_type->tp_methods = methods;
        for(const auto& method : m_cPythonMemberFunctions){
            method->AllocateObjectsTypes(m_module);
            *methods = *method->ToPython();
            methods++;
        }
        *methods = {NULL, NULL, 0, NULL};
    }

    void CPythonMetaClass::InitEnumValues() {
        PyMemberDef *enumValues = new PyMemberDef[m_cPythonEnumValues.size() + 1]; //spare space for sentinal
        m_type->tp_members = enumValues;
        for (const auto &enumValue : m_cPythonEnumValues) {
            *enumValues = *enumValue->ToPython();
            enumValues++;
        }
        *enumValues = {NULL, 0, 0, 0, NULL};
    }

    PyObject* CPythonMetaClass::InitializeSubType(const std::string& name, const std::string& doc) const
    {
        PyTypeObject* typeInstance = (PyTypeObject*)m_type->tp_alloc(m_type.get(), 0);
        new(typeInstance) PyTypeObject{
                PyVarObject_HEAD_INIT(&(*m_type), 0)
                name.c_str(),              /* tp_name */
                sizeof(PyObject),          /* tp_basicsize */
                0,                         /* tp_itemsize */
                NULL,                      /* tp_dealloc */
                0,                         /* tp_print */
                0,                         /* tp_getattr */
                0,                         /* tp_setattr */
                0,                         /* tp_compare */
                0,                         /* tp_repr */
                0,                         /* tp_as_number */
                0,                         /* tp_as_sequence */
                0,                         /* tp_as_mapping */
                0,                         /* tp_hash */
                0,                         /* tp_call */
                0,                         /* tp_str */
                0,                         /* tp_getattro */
                0,                         /* tp_setattro */
                0,                         /* tp_as_buffer */
                Py_TPFLAGS_HAVE_CLASS |
                Py_TPFLAGS_HAVE_WEAKREFS,  /* tp_flags */
                doc.c_str(),               /* tp_doc */
                0,                         /* tp_traverse */
                0,                         /* tp_clear */
                0,                         /* tp_richcompare */
                0,                         /* tp_weaklistoffset */
                0,                         /* tp_iter */
                0,                         /* tp_iternext */
                NULL,                      /* tp_methods */
                NULL,                      /* tp_members */
                0,                         /* tp_getset */
                0,                         /* tp_base */
                0,                         /* tp_dict */
                0,                         /* tp_descr_get */
                0,                         /* tp_descr_set */
                0,                         /* tp_dictoffset */
                NULL,                      /* tp_init */
                0,                         /* tp_alloc */
                NULL,                      /* tp_new */
                NULL,                      /* Low-level free-memory routine */
                &IsCollectable             /* For PyObject_IS_GC */
        };
        PyType_Ready(typeInstance);
        for(auto& enumValue : m_cPythonEnumValues)
            *(int*)((char*)typeInstance + enumValue->GetOffset()) = enumValue->GetValue();

        CPYTHON_VERIFY(PyModule_AddObject(m_module.GetModule(), name.c_str(), (PyObject*)typeInstance) == 0, "Type registration with module failed");
    }

    void CPythonMetaClass::InitStaticType() {
        {
            PyType_Ready(&m_staticType);
            Py_IncRef((PyObject*)&m_staticType);
        }
    }
}

