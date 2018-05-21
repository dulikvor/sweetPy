#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <Python.h>
#include "CPyModuleContainer.h"
#include "CPythonMember.h"
#include "ICPythonFunction.h"
#include "CPythonModule.h"
#include "TypeState.h"
#include "CPythonMetaClass.h"

namespace pycppconn {

    template<typename T, typename Type = typename std::remove_reference<T>::type, typename std::enable_if<std::__not_<std::is_pointer<Type>>::value,bool>::type = true>
    class CPythonRefObject {
    public:
        CPythonRefObject(Type& object) : m_object(object){}
        Type* operator->(){ return &m_object; } //for this usage by CPythonFunction
        const Type* operator->() const { return &m_object; } //for this usage by CPythonFunction
        Type& GetRef(){ return m_object; }
        static int Traverse(PyObject *self, visitproc visit, void *arg)
        {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //the descriptors are placed with the type it self, a descriptor per member.
            PyTypeObject *type = Py_TYPE(self);
            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_VISIT(type);

            return 0;
        }
    private:
        Type& m_object;
    };


    template<typename Type = void*>
    class CPythonRefType {
    public:
        CPythonRefType(CPythonModule &module, const std::string &name, const std::string &doc)
        : m_module(module), m_typeState(new TypeState(name, doc)) {
            m_typeState->PyType.reset(new PyTypeObject{
                    PyVarObject_HEAD_INIT(&CPythonMetaClass::GetStaticMetaType(), 0)
                    m_typeState->Name.c_str(), /* tp_name */
                    sizeof(CPythonRefObject<Type>) + sizeof(PyObject),/* tp_basicsize */
                    0,                         /* tp_itemsize */
                    0,                          /* tp_dealloc */
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
                    Py_TPFLAGS_HAVE_GC |
                    Py_TPFLAGS_HEAPTYPE,       /* tp_flags */
                    m_typeState->Doc.c_str(),  /* tp_doc */
                    &Traverse,                 /* tp_traverse */
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
            });
            Py_IncRef((PyObject *) m_typeState->PyType.get()); //Making sure the true owner of the type is CPythonClass
        }

        ~CPythonRefType()
        {
            InitMembers();
            InitMethods();
            PyType_Ready(m_typeState->PyType.get());
            CPyModuleContainer::Instance().AddType(CPyModuleContainer::TypeHash<CPythonRefType<Type>>(), m_typeState->PyType.get());
            m_module.AddType(std::move(m_typeState));
        }

        static void InitStaticType()
        {
            if(std::is_same<Type, void*>::value == false)
                throw CPythonException(PyExc_TypeError, SOURCE, "InitStaticType is not support by current type");
            PyType_Ready(&m_staticType);
        }

        static PyTypeObject& GetStaticType()
        {
            if(std::is_same<Type, void*>::value == false)
                throw CPythonException(PyExc_TypeError, SOURCE, "GetStaticType is not support by current type");
            return m_staticType;
        }

        template<typename T>
        static PyObject* StaticTypeAlloc(T&& reference)
        {
            PyObject* instance = m_staticType.tp_alloc(&m_staticType, 0);
            new(instance + 1)CPythonRefObject<T>(std::forward<T>(reference));
            return instance;
        }

        PyObject* Alloc(Type&& reference)
        {
            PyObject* instance = m_typeState->PyType->tp_alloc(m_typeState->PyType.get(), 0);
            new(instance + 1)CPythonRefObject<Type>(std::forward<Type>(reference));
            return instance;
        }

        void AddMethods(const std::vector<std::shared_ptr<ICPythonFunction>>& methods){ m_cPythonMemberFunctions = methods; }
        void AddMembers(const std::vector<std::shared_ptr<ICPythonMember>>& members){ m_cPythonMembers = members; }

        void InitMethods()
        {
            PyMethodDef *methods = new PyMethodDef[m_cPythonMemberFunctions.size() + 1]; //spare space for sentinal
            m_typeState->PyType->tp_methods = methods;
            for (const auto &method : m_cPythonMemberFunctions) {
                *methods = *method->ToPython();
                methods++;
            }
            *methods = {NULL, NULL, 0, NULL};
        }

        void InitMembers()
        {
            PyMemberDef *members = new PyMemberDef[m_cPythonMembers.size() + 1]; //spare space for sentinal
            m_typeState->PyType->tp_members = members;
            for (const auto &member : m_cPythonMembers) {
                *members = *member->ToPython();
                members++;
            }
            *members = {NULL, 0, 0, 0, NULL};
        }

    private:
        static int Traverse(PyObject *self, visitproc visit, void *arg)
        {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //the descriptors are placed with the type it self, a descriptor per member.
            PyTypeObject *type = Py_TYPE(self);
            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_VISIT(type);

            return 0;
        }

    private:
        static PyTypeObject m_staticType;
        std::vector<std::shared_ptr<ICPythonFunction>> m_cPythonMemberFunctions;
        std::vector<std::shared_ptr<ICPythonMember>> m_cPythonMembers;
        std::unique_ptr<TypeState> m_typeState;
        CPythonModule &m_module;
    };
}