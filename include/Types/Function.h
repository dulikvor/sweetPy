#pragma once

#include <Python.h>
#include <memory>
#include "core/Assert.h"
#include "../Core/Deleter.h"
#include "Tuple.h"
#include "List.h"
#include "Dictionary.h"
#include "AsciiString.h"

namespace sweetPy{
    PyCodeObject
    class CodeObject
    {
    public:
        explicit CodeObject(PyObject* object)
        {
            VERIFY(PyCode_Check(object), "Provided instance type is not  of PyCode_Type");
            PyCodeObject &codeObject = *reinterpret_cast<PyCodeObject *>(object);
            m_argCount = codeObject.co_argcount;
            m_kwOnlyArgCount = codeObject.co_kwonlyargcount;
            m_nLocals = codeObject.co_nlocals;
            m_stackSize = codeObject.co_stacksize;
            m_flags = codeObject.co_flags;
            m_firstLineno = codeObject.co_firstlineno;
            m_code = Object<std::string>::from_python(codeObject.co_code);
            m_consts.reset(new List(codeObject.co_consts));
            m_names.reset(new List(codeObject.co_name));
            m_varNames.reset(new Tuple(codeObject.co_varnames));
            m_freeVars.reset(new Tuple(codeObject.co_freevars));
            m_cellVars.reset(new Tuple(codeObject.co_cellvars));
        }
        
        int get_arg_count() const { return m_argCount; }
        int get_kw_only_arg_count() const { return m_kwOnlyArgCount; }
        int get_n_locals() const { return m_nLocals; }
        int get_stack_size() const { return m_stackSize; }
        int get_flags() const { return m_flags; }
        const std::string& get_code() const { return m_code; }
        const List& get_consts() const { return m_consts; }
        const List& get_names() const { return m_names; }
        const Tuple& get_var_names() const { return m_varNames; }
        const Tuple& get_free_vars() const { return m_freeVars; }
        const Tuple& get_cell_vars() const { return m_cellVars; }
        
    private:
        int m_argCount;
        int m_kwOnlyArgCount;
        int m_nLocals;
        int m_stackSize;
        int m_flags;
        int m_firstLineno;
        std::string m_code;
        typedef std::unique_ptr<List> CountsPtr;
        CountsPtr m_consts;
        typedef std::unique_ptr<List> NamesPtr;
        NamesPtr m_names;
        typedef std::unique_ptr<Tuple> VarNamesPtr;
        VarNamesPtr m_varNames;
        typedef std::unique_ptr<Tuple> FreeVarsPtr;
        FreeVarsPtr m_freeVars;
        typedef std::unique_ptr<Tuple> CellVarsPtr;
        CellVarPtr m_cellVars;
    };
    
    PyFunctionObject
    class Function
    {
    public:
        explicit Function(PyObject* object)
        {
            CPYTHON_VERIFY(PyFunction_Check(object), "Received object is not an instance of PyFunction");
            PyFunctionObject& function = *reinterpret_cast<PyFunctionObject*>(object);
            if(function.func_code)
                m_codeObject.reset(new CodeObject(function.func_code));
            if(function.func_globals)
                m_globals.reset(new Dictionary(function.func_globals));
            if(function.func_defaults)
                m_defaults.reset(new Tuple(function.func_defaults));
            if(function.func_kwdefaults)
                m_kwdefaults.reset(new Dictionary(function.func_kwdefaults));
            if(function.func_closure)
                m_closure.reset(new Tuple(function.func_closure));
            m_doc = Object<std::string>::from_python(function.func_doc);
            m_name = Object<std::string>::from_python(function.func_name);
        }
        
        const CodeObject& get_code_object() const { return *m_codeObject; }
        const Dictionary& get_globals() const { return *m_globals; }
        const Tuple& get_defaults() const { return *m_defaults; }
        const Dictionary& get_kwdefaults() const { return *m_kwdefaults; }
        const Tuple& get_closure() const { return *m_closure; }
        const std::string& get_doc() const { return m_doc; }
        const std::string& get_name() const { return m_name; }
        
    private:
        typedef std::unique_ptr<CodeObject> CodeObjectPtr;
        CodeObjectPtr m_codeObject;
        typedef std::unique_ptr<Dictionary> GlobalsPtr;
        GlobalsPtr m_globals;
        typedef std::unique_ptr<Tuple> DefaultsPtr;
        DefaultsPtr m_defaults;
        typedef std::unique_ptr<Dictionary> KWDefaultsPtr;
        KWDefaultsPtr m_kwdefaults;
        typedef std::unique_ptr<Tuple> ClosurePtr;
        ClosurePtr m_closure;
        std::string m_doc;
        std::string m_name;
    };
}