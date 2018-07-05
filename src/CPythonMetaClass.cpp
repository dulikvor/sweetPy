#include "CPythonMetaClass.h"
#include "CPythonModule.h"
#include "ICPythonFunction.h"
#include "CPythonEnumValue.h"
#include "CPythonGlobalFunction.h"
#include "CPythonEnum.h"

namespace sweetPy {

    template<bool IsEnumMeta>
    CPythonMetaClassType<IsEnumMeta>::CPythonMetaClassType(const std::string &name, const std::string &doc, int extendedSize)
                                                 : CPythonType(name, doc)
    {
        ob_type = &PyType_Type;
        ob_refcnt = 1;
        ob_size = 0;
        tp_name = m_name.c_str();
        tp_basicsize = (Py_ssize_t)(sizeof(PyHeapTypeObject) + extendedSize);
        tp_flags = Py_TPFLAGS_HAVE_CLASS |
                   Py_TPFLAGS_HAVE_WEAKREFS;
        tp_dealloc = &Dealloc;
        tp_doc = m_doc.c_str();
        tp_is_gc = &IsCollectable;
        tp_base = &PyType_Type;
    }

    template<bool IsEnumMeta>
    int CPythonMetaClassType<IsEnumMeta>::IsCollectable(PyObject *obj) {
        return Collectable::False;
    }

    template<bool IsEnumMeta>
    void CPythonMetaClassType<IsEnumMeta>::Dealloc(PyObject *object)
    {
        //No need to call reference forget - being called by _Py_Dealloc
        PyTypeObject* type = Py_TYPE(object);

        if (PyType_IS_GC(type))
            PyObject_GC_UnTrack(object);

        if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
            Py_DECREF(type);

        if(IsEnumMeta)
            ((CPythonEnumType*)(object + 1))->~CPythonEnumType();
        type->tp_free(object);
    }

    static int IsCollectable(PyObject *obj) {
        return CPythonMetaClassType<false>::Collectable::False;
    }

    template<bool IsEnumMeta>
    PyTypeObject CPythonMetaClass<IsEnumMeta>::m_staticType = {
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

    template<bool IsEnumMeta>
    CPythonMetaClass<IsEnumMeta>::CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc, int extendedSize)
            :m_module(module), m_type(new CPythonMetaClassType<IsEnumMeta>(name, doc, extendedSize)){
        Py_IncRef((PyObject*)m_type.get());
    }

    template<bool IsEnumMeta>
    CPythonMetaClass<IsEnumMeta>::~CPythonMetaClass()
    {
        m_module.AddType(std::move(m_type));
    }

    template<bool IsEnumMeta>
    PyTypeObject& CPythonMetaClass<IsEnumMeta>::GetStaticMetaType() {
        return m_staticType; }

    template<bool IsEnumMeta>
    PyTypeObject& CPythonMetaClass<IsEnumMeta>::ToPython() const{
        return *m_type;
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::SetCallableOperator(ternaryfunc function)
    {
       m_type->tp_call = function;
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::InitType()
    {
        InitMethods();
        InitEnumValues();
        PyType_Ready(m_type.get());
    }


    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::AddMethod(const std::shared_ptr<ICPythonFunction>& method){
        //No need to register the CPythonFunction with in the function contaier, was already registered as static
        //function by the CPythonClass who created this meta class type.
        m_cPythonMemberFunctions.emplace_back(method);
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::AddEnumValue(std::unique_ptr<CPythonEnumValue>&& enumValue)
    {
        m_cPythonEnumValues.emplace_back(std::move(enumValue));
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::InitMethods(){
        if(m_cPythonMemberFunctions.empty() == false)
        {
            PyMethodDef* methods = new PyMethodDef[m_cPythonMemberFunctions.size() + 1]; //spare space for sentinal
            m_type->tp_methods = methods;
            for(const auto& method : m_cPythonMemberFunctions){
                method->AllocateObjectsTypes(m_module);
                *methods = method->ToPython()->MethodDef;
                methods++;
            }
            *methods = {NULL, NULL, 0, NULL};
        }
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::InitEnumValues() {
        if(m_cPythonEnumValues.empty() == false)
        {
            PyMemberDef *enumValues = new PyMemberDef[m_cPythonEnumValues.size() + 1]; //spare space for sentinal
            m_type->tp_members = enumValues;
            for (const auto &enumValue : m_cPythonEnumValues) {
                *enumValues = *enumValue->ToPython();
                enumValues++;
            }
            *enumValues = {NULL, 0, 0, 0, NULL};
        }
    }

    template<bool IsEnumMeta>
    PyObject* CPythonMetaClass<IsEnumMeta>::InitializeEnumType(const std::string& name, const std::string& doc) const
    {
        PyTypeObject* type = (PyTypeObject*)m_type->tp_alloc(m_type.get(), 0);
        new(type)CPythonEnumType(name, doc, m_type.get());
        PyType_Ready(type);
        for(auto& enumValue : m_cPythonEnumValues)
            *(int*)((char*)type + enumValue->GetOffset()) = enumValue->GetValue();

        CPYTHON_VERIFY(PyModule_AddObject(m_module.GetModule(), name.c_str(), (PyObject*)type) == 0, "Type registration with module failed");
    }

    template<bool IsEnumMeta>
    PyObject* CPythonMetaClass<IsEnumMeta>::InitializeFunctionType(const std::string& name, const std::string& doc) const
    {
        PyTypeObject* type = (PyTypeObject*)m_type->tp_alloc(m_type.get(), 0);
        new(type)CPythonFunctionType(name, doc, m_type.get());
        PyType_Ready(type);
        CPYTHON_VERIFY(PyModule_AddObject(m_module.GetModule(), name.c_str(), (PyObject*)type) == 0, "Type registration with module failed");
    }

    template<bool IsEnumMeta>
    void CPythonMetaClass<IsEnumMeta>::InitStaticType() {
        {
            PyType_Ready(&m_staticType);
            Py_IncRef((PyObject*)&m_staticType);
        }
    }

    template class CPythonMetaClass<true>;
    template class CPythonMetaClass<false>;
}

