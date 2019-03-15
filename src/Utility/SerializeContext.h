#pragma once

#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include "flatbuffers/flatbuffers.h"
#include "Types_generated.h"
#include "../Types/Tuple.h"
#include "SerializeTypes.h"

namespace sweetPy
{
    template<SerializeType::Enumeration> class SweetPickleImpl;
    
    namespace flat_traits
    {
        template<typename T> struct TypeToTypeId{};
        template<> struct TypeToTypeId<serialize::Short> : std::integral_constant<serialize::all_types, serialize::all_types::Short>{};
        template<> struct TypeToTypeId<serialize::Int> : std::integral_constant<serialize::all_types, serialize::all_types::Int>{};
        template<> struct TypeToTypeId<serialize::Bool> : std::integral_constant<serialize::all_types, serialize::all_types::Bool>{};
        template<> struct TypeToTypeId<serialize::Double> : std::integral_constant<serialize::all_types, serialize::all_types::Double>{};
        template<> struct TypeToTypeId<serialize::String> : std::integral_constant<serialize::all_types, serialize::all_types::String>{};
        template<> struct TypeToTypeId<serialize::Tuple> : std::integral_constant<serialize::all_types, serialize::all_types::Tuple>{};
        template<> struct TypeToTypeId<serialize::List> : std::integral_constant<serialize::all_types, serialize::all_types::List>{};
    }
    
    class SerializeContext
    {
    public:
        typedef  void(*_Deallocate)(char*);
        typedef std::unique_ptr<char, _Deallocate> _Buffer;
        typedef std::pair<_Buffer, std::size_t> String;
        virtual ~SerializeContext(){}
        virtual String Finish(bool shallowCopy = true) = 0;
        
    protected:
        static void deallocate(char* ptr){ delete[] ptr; }
    };
    
    template<SerializeType::Enumeration type>
    class ConcreteSerializeContext : SerializeContext
    {
    public:
        virtual ~ConcreteSerializeContext(){}
        String Finish(bool shallowCopy = true) override
        {
            throw core::Exception(__CORE_SOURCE, "Method not supported");
        }
    };
    
    template<>
    class ConcreteSerializeContext<SerializeType::FlatBuffers> : public SerializeContext
    {
    public:
        String Finish(bool shallowCopy = true) override
        {
            std::vector<flatbuffers::Offset<serialize::Object>> objects;
            for(auto& offsetPair : m_offsets)
                objects.emplace_back(serialize::CreateObject(m_builder, offsetPair.first, offsetPair.second));
        
            auto offsetVec = m_builder.CreateVector(objects);
            auto offset = serialize::CreateObjects(m_builder, offsetVec);
            
            m_builder.Finish(offset);
    
            char* _ptr;
            std::size_t _size = m_builder.GetSize();
            if(shallowCopy)
                _ptr = reinterpret_cast<char*>(m_builder.GetBufferPointer());
            else
            {
                _ptr = new char[_size];
                std::memcpy(_ptr, reinterpret_cast<char*>(m_builder.GetBufferPointer()), _size);
            }
            
            String ret(std::make_pair(
                    _Buffer(_ptr, shallowCopy ? [](char*){} : &deallocate),
                    _size
            ));
            return ret;
        }
        
    private:
        friend SweetPickleImpl<SerializeType::FlatBuffers>;
        typedef flatbuffers::Offset<void> UnionOffset;
        typedef serialize::all_types Type;
        
        class const_iterator;
        
        class Object
        {
        public:
            template<typename T>
            typename std::enable_if<std::is_same<serialize::Container,
                typename std::decay<T>::type>::value == false, const T&>::type Get() const
            {
                if(m_object == nullptr || m_object->object_type() != flat_traits::TypeToTypeId<T>::value)
                    throw core::Exception(__CORE_SOURCE, "Requested type dosen't match");
                return *reinterpret_cast<T const *>(m_object->object());
            }
    
            template<typename T>
            typename std::enable_if<std::is_same<serialize::Container,
                    typename std::decay<T>::type>::value, const T&>::type Get() const
            {
                if(m_object == nullptr || (m_object->object_type() != flat_traits::TypeToTypeId<serialize::Tuple>::value
                 && m_object->object_type() != flat_traits::TypeToTypeId<serialize::List>::value))
                    throw core::Exception(__CORE_SOURCE, "Requested type dosen't match");
                return *reinterpret_cast<serialize::Container const *>(m_object->object());
            }
            
            serialize::all_types GetType() const
            {
                return m_object->object_type();
            }
            
        private:
            friend const_iterator;
            Object(){}
            Object(serialize::Object* object):m_object(object){}
            Object& operator=(serialize::Object* object){m_object = object; return *this;};
            
        private:
            serialize::Object* m_object;
        };
    
        class const_iterator : public std::iterator<std::forward_iterator_tag, Object* const, std::ptrdiff_t, const Object*, const Object&>
        {
        private:
            friend ConcreteSerializeContext<SerializeType::FlatBuffers>;
            typedef flatbuffers::Vector<flatbuffers::Offset<serialize::Object>> ObjectsVector;
            explicit const_iterator(ObjectsVector::const_iterator it, ObjectsVector::const_iterator end)
                : m_it(it), m_end(end), m_currentObject(const_cast<serialize::Object*>(*it)){}
            
        public:
            const_iterator& operator++()
            {
                m_it++;
                if(m_it == m_end)
                    m_currentObject = nullptr;
                else
                    m_currentObject = const_cast<serialize::Object*>(*m_it);
                
                return *this;
            }
            bool operator==(const const_iterator& other) const{return m_it == other.m_it;}
            bool operator!=(const const_iterator& other) const{return m_it != other.m_it;}
            reference operator*() const{return m_currentObject;}
            pointer operator->() const{return &m_currentObject;}
    
        private:
            ObjectsVector::const_iterator m_it;
            ObjectsVector::const_iterator m_end;
            Object m_currentObject;
        };
    
        const_iterator StartRead(const String& buffer)
        {
            const serialize::Objects& objects = *serialize::GetObjects(reinterpret_cast<const void*>(buffer.first.get()));
            return const_iterator(objects.objects()->begin(), objects.objects()->end());
        }
        
        flatbuffers::FlatBufferBuilder& GetBuilder() { return m_builder; }
        void AddOffset(Type type, const UnionOffset& offset){ m_offsets.emplace_back(std::make_pair(type, offset)); }
        
    private:
        flatbuffers::FlatBufferBuilder m_builder;
        std::vector<std::pair<Type, UnionOffset>> m_offsets;
    };
}
