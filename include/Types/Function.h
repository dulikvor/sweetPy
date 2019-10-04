#pragma once

#include <Python.h>
#include "core/Assert.h"
#include "../Core/Deleter.h"
#include "Tuple.h"
#include "List.h"
#include "AsciiString.h"

namespace sweetPy{
    class CodeObject
    {
    private:
    #define CodeObjectPtr(object)\
    reinterpret_cast<PyCodeObject*>(m_codeObject.get())
        
        explicit CodeObject(PyObject* codeObject)
            :m_codeObject(nullptr, &Deleter::Owner)
        {
            VERIFY(codeObject->ob_type == &PyCode_Type, "Provided instance is not a code object instance");
            Py_INCREF(codeObject);
            m_codeObject.reset(codeObject);
        }
        
        int ArgCount() const {return CodeObjectPtr(m_codeObject)->co_argcount;}
        int KeyWordOnlyCount() const {return CodeObjectPtr(m_codeObject)->co_kwonlyargcount;}
        int NLocals() const {return CodeObjectPtr(m_codeObject)->co_nlocals;}
        int Flags() const {return CodeObjectPtr(m_codeObject)->co_flags;}
        int FirstLineNo() const {return CodeObjectPtr(m_codeObject)->co_firstlineno;}
        std::string Code() const;
        List Consts() const;
        List Names() const;
        Tuple VarNames() const;
        Tuple FreeVars() const;
        Tuple CellVars() const;
        AsciiString FileName() const;
        AsciiString Name() const;
        std::string LNoTab() const;
        
    private:
        object_ptr m_codeObject;
    };
}