#pragma once

#include <type_traits>
#include <utility>
#include <Python.h>
#include "CPyModuleContainer.h"

namespace pycppconn {

    template<typename T, typename Type = typename std::remove_reference<T>::type, typename std::enable_if<std::__not_<std::is_pointer<Type>>::value,bool>::type = true>
    class CPythonRefObject {
    public:
        CPythonRefObject(Type& object) : m_object(object){}
        Type& GetRef(){
            return m_object;
        }
        static int Traverse(PyObject *self, visitproc visit, void *arg) {
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

    class CPythonRefType {
    public:
        static void InitStaticType();
        template<typename T>
        static PyObject* Alloc(T&& reference){
            PyObject* instance = m_staticType.tp_alloc(&m_staticType, 0);
            new(instance + 1)CPythonRefObject<T>(std::forward<T>(reference));
            return instance;
        }
        static PyTypeObject& GetType();

    private:
        static PyTypeObject m_staticType;
    };
}