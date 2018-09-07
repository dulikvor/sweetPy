#include "Dictionary.h"
#include "Exception.h"
#include "Assert.h"

namespace sweetPy{
    Dictionary::Dictionary(PyObject* const object)
        :m_dict(nullptr, &Deleter::Borrow)
    {
        if(object->ob_type == &PyModule_Type)
            m_dict.reset(PyModule_GetDict(object));
        else
            m_dict.reset(((PyTypeObject*)object->ob_type)->tp_dict);
    }

    object_ptr Dictionary::GetObject(PyObject* key)
    {
       return object_ptr(PyDict_GetItem(m_dict.get(), key), &Deleter::Borrow);
    }

    std::string Dictionary::GetObjectKey(PyObject* const object)
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(m_dict.get(), &pos, &key, &value))
        {
            if(value == object)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(key), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                return std::string(PyBytes_AsString(bytesObject.get()));
            }

        }
        throw CPythonException(PyExc_Exception, __CORE_SOURCE, "Object was never found");
    }

    bool Dictionary::IsObjectExists(PyObject* const object)
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(m_dict.get(), &pos, &key, &value))
        {
            if(value == object)
            {
                return true;
            }
        }
        return false;
    }
}
