#include "CPythonMetaClass.h"
#include "CPythonModule.h"
#include "CPythonGlobalFunction.h"

namespace sweetPy {

    CPythonMetaClassType::CPythonMetaClassType(const std::string &name, const std::string &doc, int extendedSize)
                                                 : CPythonType(name, doc)
    {
        ob_base.ob_base.ob_type = &PyType_Type;
        ob_base.ob_base.ob_refcnt = 1;
        ob_base.ob_size = 0;
        tp_name = m_name.c_str();
        tp_basicsize = (Py_ssize_t)(sizeof(PyHeapTypeObject) + extendedSize);
        tp_dealloc = &Dealloc;
        tp_doc = m_doc.c_str();
        tp_base = &CPythonMetaClass::GetStaticMetaType();
    }

    int CPythonMetaClassType::IsCollectable(PyObject *obj) {
        return Collectable::False;
    }

    void CPythonMetaClassType::Dealloc(PyObject *object)
    {
        //No need to call reference forget - being called by _Py_Dealloc
        PyTypeObject* type = Py_TYPE(object);

        if (PyType_IS_GC(type))
            PyObject_GC_UnTrack(object);

        if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
            Py_DECREF(type);

        type->tp_free(object);
    }
    
    static int IsCollectable(PyObject *obj) {
        return CPythonMetaClassType::Collectable::False;
    }

    CPythonMetaClass::NonCollectableMetaType::NonCollectableMetaType()
            :PyTypeObject{}
    {
        ob_base.ob_base.ob_type = &PyType_Type;
        ob_base.ob_base.ob_refcnt = 1;
        tp_name = "CPythonMeta";
        tp_basicsize = sizeof(PyObject);
        tp_doc = "CPython meta class";
        tp_base = &PyType_Type;
        tp_is_gc = &IsCollectable;
        tp_dealloc = &Dealloc;
        tp_free = &Free;
    }

    typename CPythonMetaClass::NonCollectableMetaType CPythonMetaClass::m_staticType{};
    
    void CPythonMetaClass::NonCollectableMetaType::Dealloc(PyObject* object)
    {
        PyTypeObject& cls = *reinterpret_cast<PyTypeObject*>(object);
        _Py_ForgetReference(object);
        Py_XDECREF(cls.tp_dict);
        Py_XDECREF(cls.tp_base);
        Py_XDECREF(cls.tp_bases);
        Py_XDECREF(cls.tp_mro);
        Py_TYPE(object)->tp_free(object);
    }
    
    void CPythonMetaClass::NonCollectableMetaType::Free(void* object)
    {
        delete reinterpret_cast<PyObject*>(object);
    }

    CPythonMetaClass::CPythonMetaClass(CPythonModule& module, const std::string& name, const std::string& doc, int extendedSize)
            :m_module(module), m_type((PyObject*)new CPythonMetaClassType(name, doc, extendedSize), &Deleter::Owner){
        Py_IncRef((PyObject*)m_type.get());
    }

    CPythonMetaClass::~CPythonMetaClass()
    {
        m_module.AddType((CPythonType*)m_type.get());
        CPyModuleContainer::Instance().AddType(std::hash<std::string>{}(((CPythonType*)m_type.get())->GetName()), std::move(m_type));
    }

    PyTypeObject& CPythonMetaClass::GetStaticMetaType() {
        return m_staticType; }

    PyTypeObject& CPythonMetaClass::ToPython() const{
        return *(PyTypeObject*)m_type.get();
    }

    void CPythonMetaClass::InitType()
    {
        PyType_Ready((PyTypeObject*)m_type.get());
        InitStaticMethods();
    }


    void CPythonMetaClass::AddStaticMethod(const std::shared_ptr<CPythonFunction>& staticMethod)
    {
        //No need to register the CPythonFunction with in the function container, was already registered as static
        //function by the CPythonClass who created this meta class type.
        m_staticMethods.emplace_back(staticMethod);
    }

    void CPythonMetaClass::InitStaticMethods()
    {
        if(m_staticMethods.empty() == false)
        {
            for(const auto& method : m_staticMethods)
            {
                method->AllocateTypes(m_module);
                auto descriptor = method->ToPython();

                object_ptr name(PyUnicode_FromString(descriptor->ml_name), &Deleter::Owner);
                object_ptr cFunction(PyCFunction_NewEx(descriptor.get(), name.get(), NULL), &Deleter::Owner);

                PyDict_SetItem(((PyTypeObject*)m_type.get())->tp_dict, name.get(), cFunction.get());

                ((CPythonType*)m_type.get())->AddDescriptor(std::move(descriptor));
            }
        }
    }

    void CPythonMetaClass::InitStaticType() {
        {
            PyType_Ready(&m_staticType);
            Py_IncRef((PyObject*)&m_staticType);
        }
    }
}

