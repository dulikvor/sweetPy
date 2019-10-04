#include "Types/Function.h"
#include "Detail/CPythonObject.h"

namespace sweetPy{
    std::string CodeObject::Code() const
    {
        return Object<std::string>::FromPython(CodeObjectPtr(m_codeObject)->co_code);
    }
    
    List CodeObject::Consts() const
    {
        return Object<List>::FromPython(CodeObjectPtr(m_codeObject)->co_consts);
    }
    
    List CodeObject::Names() const
    {
        return Object<List>::FromPython(CodeObjectPtr(m_codeObject)->co_names);
    }
    
    Tuple CodeObject::VarNames() const
    {
        return Object<Tuple>::FromPython(CodeObjectPtr(m_codeObject)->co_varnames);
    }
    
    Tuple CodeObject::FreeVars() const
    {
        return Object<Tuple>::FromPython(CodeObjectPtr(m_codeObject)->co_freevars);
    }
    
    Tuple CodeObject::CellVars() const
    {
        return Object<Tuple>::FromPython(CodeObjectPtr(m_codeObject)->co_cellvars);
    }
    
    AsciiString CodeObject::FileName() const
    {
        return Object<AsciiString>::FromPython(CodeObjectPtr(m_codeObject)->co_filename);
    }
    
    AsciiString CodeObject::Name() const
    {
        return Object<AsciiString>::FromPython(CodeObjectPtr(m_codeObject)->co_name);
    }
    
    std::string CodeObject::LNoTab() const
    {
        return Object<std::string>::FromPython(CodeObjectPtr(m_codeObject)->co_lnotab);
    }
}