#pragma once

#include <Python.h>
#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <iterator>
#include <vector>
#include <algorithm>
#include <structmember.h>

namespace sweetPy {
    
    struct CPythonGCHead
    {
        PyGC_Head m_gc;
    };

    //vptr is not allowed
    struct CPythonType : public CPythonGCHead, public PyHeapTypeObject
    {
    public:
        typedef std::unique_ptr<PyMethodDef> DescriptorPtr;
        typedef std::function<void(PyObject*)> Free;
        template<typename T> struct CPythonTypeHash{};
        template<typename FreeT>
        CPythonType(const std::string& name, const std::string& doc, std::size_t hash_code, const FreeT& freeType)
                :CPythonGCHead{}, PyHeapTypeObject{}, m_name(name), m_doc(doc), m_hash_code(hash_code), m_freeType(freeType)
        {
            reset_gc_head();
        }
        ~CPythonType()
        {
            if(ht_type.tp_members != nullptr)
            {
                MembersDefs membersDefs(ht_type.tp_members);
                for(auto& memberDef : membersDefs)
                {
                    delete[] memberDef.name;
                    delete[] memberDef.doc;
                }
            }
            delete[] ht_type.tp_members;
            delete[] ht_type.tp_methods;
        }
        void add_descriptor(DescriptorPtr&& descriptor)
        {
            m_descriptors.emplace_back(descriptor.release());
        }
        const std::string& get_name() const {return m_name;}
        const std::string& get_doc() const {return m_doc;}
        std::size_t get_hash_code() const {return m_hash_code;}
        Free get_free() const {return m_freeType;}
        static PyObject* get_py_object(CPythonType* type)
        {
            return reinterpret_cast<PyObject*>(static_cast<CPythonGCHead*>(type) + 1);
        }
        static CPythonType* get_type(PyObject* object)
        {
            return static_cast<CPythonType*>(reinterpret_cast<CPythonGCHead*>(object) - 1);
        }
        void clear_trace_ref()
        {
#ifdef Py_TRACE_REF
            if(ht_type.ob_base.ob_base._ob_prev)
                ht_type.ob_base.ob_base._ob_prev->_ob_next = ht_type.ob_base.ob_base._ob_next;
            if(ht_type.ob_base.ob_base._ob_next)
                ht_type.ob_base.ob_base._ob_next->_ob_prev = ht_type.ob_base.ob_base._ob_prev;
        
            ht_type.ob_base.ob_base._ob_next = NULL;
            ht_type.ob_base.ob_base._ob_prev = NULL;
#endif
        }

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
        
    private:
        void reset_gc_head()
        {
            auto& gc = static_cast<CPythonGCHead&>(*this);
            gc.m_gc._gc_next = (uintptr_t)&m_nextStub;
            gc.m_gc._gc_prev = (uintptr_t)&m_prevStub;
        }

    protected:
        static PyGC_Head m_nextStub;
        static PyGC_Head m_prevStub;
        std::string m_name;
        std::string m_doc;
        std::size_t m_hash_code;
        typedef std::vector<DescriptorPtr> Descriptors;
        Descriptors m_descriptors; //No ownership of name and doc (functions take ownership on their meta data and not the descriptor). PyCFunctionObject won't take ownership on the descriptor, so the ownership is transfered to sweetPy CPythonType.
        Free m_freeType;
    };
}
