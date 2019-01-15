#pragma once

#include <Python.h>
#include <iostream>
#include <string>
#include <memory>
#include <iterator>
#include <vector>
#include <algorithm>
#include <structmember.h>

namespace sweetPy {

    //vptr is not allowed
    struct CPythonType : public PyTypeObject
    {
    public:
        template<typename T>
        struct CPythonTypeHash{};
        
        CPythonType(const std::string& name, const std::string& doc)
                :PyTypeObject{}, m_name(name), m_doc(doc){}
        ~CPythonType()
        {
            if(tp_members != nullptr)
            {
                MembersDefs membersDefs(tp_members);
                for(auto& memberDef : membersDefs)
                {
                    delete[] memberDef.name;
                    delete[] memberDef.doc;
                }
            }
            delete[] tp_members;
            delete[] tp_methods;
        }
        void AddDescriptor(std::unique_ptr<PyMethodDef>&& descriptor){m_descriptors.emplace_back(descriptor.release());}

        const std::string& GetName() const {return m_name;}
        const std::string& GetDoc() const {return m_doc;}

    protected:
        class MembersDefs
        {
        public:
            explicit MembersDefs(PyMemberDef* membersDefs): m_membersDefs(membersDefs){}

            class iterator : public std::iterator<std::forward_iterator_tag, PyMemberDef*, ptrdiff_t, PyMemberDef*, PyMemberDef&>
            {
            public:
                explicit iterator(PyMemberDef* memberDef): m_memberDef(memberDef ? memberDef : &m_sentinal){}
                iterator& operator++(){m_memberDef++; return *this;}
                bool operator==(const iterator& other) const{return m_memberDef == other.m_memberDef || m_memberDef->name == m_sentinal.name;} //sentinal is not actually the original sentinal, only identical by its data.
                bool operator!=(const iterator& other) const{return m_memberDef != other.m_memberDef && m_memberDef->name != m_sentinal.name;} //sentinal is not actually the original sentinal, only identical by its data.
                reference operator*() const{return *m_memberDef;}
                pointer operator->() const{return m_memberDef;}

            private:
                PyMemberDef* m_memberDef;
            };

            iterator begin(){return iterator(m_membersDefs);}
            iterator end(){return iterator(&m_sentinal);}

        private:
            PyMemberDef* m_membersDefs;
            static PyMemberDef m_sentinal;
        };

    protected:
        std::string m_name;
        std::string m_doc;
        std::vector<std::unique_ptr<PyMethodDef>> m_descriptors; //No ownership of name and doc (functions take ownership on their meta data and not the descriptor). PyCFunctionObject won't take ownership on the descriptor, so the ownership is transfered to sweetPy CPythonType.
    };

}
