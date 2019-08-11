#pragma once

#include <Python.h>
#include <memory>
#include <string>

namespace sweetPy {
    class Function
    {
    public:
        typedef std::unique_ptr<PyMethodDef> MethodDefPtr;
        Function(const std::string& name, const std::string& doc)
            :m_name(name), m_doc(doc), m_hash_code(std::hash<std::string>()(name))
        {
        }
        virtual ~Function() = default;
        Function(Function &) = delete;
        Function &operator=(Function &) = delete;
        Function(Function &&obj) = default;
        Function &operator=(Function &&obj) = default;
        virtual MethodDefPtr to_python() const = 0;
        std::size_t get_hash_code() const {return m_hash_code;}

    protected:
        std::string m_name;
        std::string m_doc;
        std::size_t m_hash_code;
    };
}
