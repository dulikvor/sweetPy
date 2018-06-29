#include "CPythonRef.h"

namespace sweetPy{

    static int Traverse(PyObject *self, visitproc visit, void *arg)
    {
        //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
        //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
        return 0;
    }

    template<>
    PyTypeObject CPythonRef<>::m_staticType = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "CPythonRefType",          /* tp_name */
            sizeof(PyObject) + sizeof(CPythonRefObject<int>),/* tp_basicsize */
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
            Py_TPFLAGS_HAVE_GC |
            Py_TPFLAGS_HAVE_WEAKREFS,  /* tp_flags */
            "CPthon meta class",       /* tp_doc */
            &Traverse,/* tp_traverse */
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
            NULL,                      /* For PyObject_IS_GC */
    };
}

