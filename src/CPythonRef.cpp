#include "CPythonRef.h"

namespace sweetPy{

    static int Traverse(PyObject *self, visitproc visit, void *arg)
    {
        //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
        //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
        return 0;
    }

    template<>
    CPythonRef<>::NonCallableRefType::NonCallableRefType()
        :PyTypeObject{}
    {
            ob_base.ob_base.ob_type = &PyType_Type;
            ob_base.ob_base.ob_refcnt = 1;
            tp_name = "CPythonRefType";
            tp_basicsize = sizeof(PyObject) + sizeof(CPythonRefObject<int>);
            tp_flags = Py_TPFLAGS_HAVE_GC;
            tp_doc = "CPython Ref Type";
            tp_traverse = &Traverse;
    }

    template<> CPythonRef<>::NonCallableRefType CPythonRef<>::m_staticType{};
}

