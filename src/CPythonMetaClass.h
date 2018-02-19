#pragma once

#include <Python.h>

namespace pycppconn {

    class CPythonMetaClass {
    public:
        enum Collectable {
            False = 0,
            True = 1
        };

        static PyTypeObject &GetMetaType() { return m_type; }
        static void InitType();

    private:
        static int IsCollectable(PyObject *obj) {
            return Collectable::False;
        }

    private:
        static PyTypeObject m_type;
    };
}


