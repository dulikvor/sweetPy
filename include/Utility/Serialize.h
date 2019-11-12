#pragma once

#include <Python.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include "../Core/Deleter.h"
#include "../Detail/CPythonObject.h"
#include "../Types/ObjectPtr.h"
#include "../Types/Container.h"
#include "../Types/Tuple.h"
#include "../Types/List.h"
#include "../Types/Dictionary.h"
#include "../Types/AsciiString.h"
#include "flatbuffers/flatbuffers.h"
#include "Types_generated.h"
#include "SerializeTypes.h"
#include "SerializeContext.h"

namespace sweetPy
{
    class SweetPickle
    {
    public:
        virtual ~SweetPickle(){}
        virtual void write(SerializeContext& context, const ObjectPtr& object) const = 0;
        virtual void write(SerializeContext& context, int value) const = 0;
        virtual void write(SerializeContext& context, double value) const = 0;
        virtual void write(SerializeContext& context, const char* value) const = 0;
        virtual void write(SerializeContext& context, const std::string& value) const = 0;
        virtual void write(SerializeContext& context, bool value) const = 0;
        virtual void write(SerializeContext& context, const Tuple& value) const = 0;
        virtual void write(SerializeContext& context, const List& value) const = 0;
        virtual void write(SerializeContext& context, const Dictionary& value) const = 0;
        virtual void start_read(SerializeContext& context, const SerializeContext::String& buffer) = 0;
        virtual void start_read(SerializeContext& context, char* buffer, std::size_t size) = 0;
        virtual void read(int& value) = 0;
        virtual void read(double& value) = 0;
        virtual void read(bool& value) = 0;
        virtual void read(std::string& value) = 0;
        virtual void read(Tuple& value) = 0;
        virtual void read(List& value) = 0;
        virtual void read(Dictionary& value) = 0;
        virtual serialize::all_types get_type() const = 0;
    };
    
    template<SerializeType::Enumeration>
    class SweetPickleImpl : public SweetPickle
    {
    public:
        virtual ~SweetPickleImpl(){}
        void write(SerializeContext& context, const ObjectPtr& object) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void write(SerializeContext& context, int value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void write(SerializeContext& context, double value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void write(SerializeContext& context, const char* value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void write(SerializeContext& context, const std::string& value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
        
        void write(SerializeContext& context, bool value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void write(SerializeContext& context, const Tuple& value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
    
        void start_read(SerializeContext& context, const SerializeContext::String& buffer) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void start_read(SerializeContext& context, char* buffer, std::size_t size) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(int& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(double& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(bool& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(std::string& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(Tuple& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(List& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void read(Dictionary& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        serialize::all_types get_type() const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    };
    
    template<>
    class SweetPickleImpl<SerializeType::FlatBuffers> : public SweetPickle
    {
    public:
        virtual ~SweetPickleImpl(){}
        void write(SerializeContext& context, const ObjectPtr& object) const override;
        void write(SerializeContext& context, int value) const override;
        void write(SerializeContext& context, double value) const override;
        void write(SerializeContext& context, const char* value) const override;
        void write(SerializeContext& context, const std::string& value) const override;
        void write(SerializeContext& context, bool value) const override;
        void write(SerializeContext& context, const Tuple& value) const override;
        void write(SerializeContext& context, const List& value) const override;
        void write(SerializeContext& context, const Dictionary& value) const override;
        void start_read(SerializeContext& context, const SerializeContext::String& buffer) override;
        void start_read(SerializeContext& context, char* buffer, std::size_t size) override;
        void read(int& value) override;
        void read(double& value) override;
        void read(bool& value) override;
        void read(std::string& value) override;
        void read(Tuple& value) override;
        void read(List& value) override;
        void read(Dictionary& value) override;
        serialize::all_types get_type() const override;
        
    private:
        flatbuffers::Offset<serialize::Container> write(SerializeContext& context, const _Container& value) const;
        typedef std::pair<flatbuffers::Offset<void>, serialize::all_types> DictionaryElementOffset;
        DictionaryElementOffset write(SerializeContext& context, const Detail::ElementValue& value) const;
        typedef flatbuffers::Vector<flatbuffers::Offset<serialize::ContainerParam>> ContainerParams;
        void read(const ContainerParams& params, _Container& value);
        ObjectPtr read_dictionary_key(const serialize::DictionaryPair& pair);
        ObjectPtr read_dictionary_value(const serialize::DictionaryPair& pair);
        static serialize::integral_types all_to_integral(const serialize::all_types allTypes);
        
    private:
        typedef ConcreteSerializeContext<SerializeType::FlatBuffers> _Context;
        std::unique_ptr<_Context::const_iterator> m_it;
    };
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, int value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateInt(builder, value);
        flatContext.add_offset(serialize::all_types::all_types_Int, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, double value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateDouble(builder, value);
        flatContext.add_offset(serialize::all_types::all_types_Double, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const char* value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateCTypeStringDirect(builder, value);
        flatContext.add_offset(serialize::all_types::all_types_CTypeString, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const std::string& value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto strOffset = builder.CreateString(value);
        auto offset = serialize::CreateString(builder, strOffset);
        flatContext.add_offset(serialize::all_types::all_types_String, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, bool value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateBool(builder, value);
        flatContext.add_offset(serialize::all_types::all_types_Bool, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const Tuple& value) const
    {
        auto containerOffset = write(context, static_cast<const _Container&>(value));
    
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateTuple(builder, containerOffset);
        flatContext.add_offset(serialize::all_types::all_types_Tuple, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const List& value) const
    {
        auto containerOffset = write(context, static_cast<const _Container&>(value));
        
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        auto offset = serialize::CreateList(builder, containerOffset);
        flatContext.add_offset(serialize::all_types::all_types_List, offset.Union());
    }
    
    
    SweetPickleImpl<SerializeType::FlatBuffers>::DictionaryElementOffset
        SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context,
                const Detail::ElementValue& value) const
    {
        typedef SweetPickleImpl<SerializeType::FlatBuffers>::DictionaryElementOffset ElementOffset;
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        PyObject* object = value.get<PyObject*>();
        
        if(PyLong_CheckExact(object) ||
                ClazzObject<ReferenceObject<int>>::is_ref(object) ||
                ClazzObject<ReferenceObject<const int>>::is_ref(object))
        {
            auto offset = serialize::CreateInt(builder, value.get<int>());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_Int);
        }
        else if(PyFloat_CheckExact(object) ||
            ClazzObject<ReferenceObject<double>>::is_ref(object) ||
            ClazzObject<ReferenceObject<const double>>::is_ref(object))
        {
            auto offset = serialize::CreateDouble(builder, value.get<double>());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_Double);
        }
        else if(PyBool_Check(object))
        {
            auto offset = serialize::CreateBool(builder, value.get<bool>());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_Bool);
        }
        else if(PyBytes_CheckExact(object) ||
                ClazzObject<ReferenceObject<std::string>>::is_ref(object) ||
                ClazzObject<ReferenceObject<const std::string>>::is_ref(object))
        {
            auto offset = serialize::CreateStringDirect(builder, value.get<std::string>().c_str());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_String);
        }
        else if(PyUnicode_CheckExact(object))
        {
            auto offset = serialize::CreateCTypeStringDirect(builder, value.get<std::string>().c_str());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_CTypeString);
        }
        else if(ClazzObject<ReferenceObject<AsciiString>>::is_ref(object) ||
                ClazzObject<ReferenceObject<const AsciiString>>::is_ref(object))
        {
            auto offset = serialize::CreateCTypeStringDirect(builder, value.get<AsciiString>().get_str().c_str());
            return ElementOffset(offset.Union(), serialize::all_types::all_types_CTypeString);
        }
        else if(PyList_CheckExact(object) ||
                ClazzObject<ReferenceObject<List>>::is_ref(object) ||
                ClazzObject<ReferenceObject<const List>>::is_ref(object))
        {
            auto containerOffset = write(context, static_cast<const _Container&>(value.get<List>()));
            auto offset = serialize::CreateList(builder, containerOffset);
            return ElementOffset(offset.Union(), serialize::all_types::all_types_List);
        }
        else if(PyTuple_CheckExact(object) ||
                ClazzObject<ReferenceObject<Tuple>>::is_ref(object) ||
                ClazzObject<ReferenceObject<const Tuple>>::is_ref(object))
        {
            auto containerOffset = write(context, static_cast<const _Container&>(value.get<Tuple>()));
            auto offset = serialize::CreateTuple(builder, containerOffset);
            return ElementOffset(offset.Union(), serialize::all_types::all_types_Tuple);
        }
        else
            throw core::Exception(__CORE_SOURCE, "Non supported types");
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const Dictionary& value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        std::vector<flatbuffers::Offset<serialize::DictionaryPair>> params;
        typedef SweetPickleImpl<SerializeType::FlatBuffers>::DictionaryElementOffset ElementOffset;
        
        auto storeElement = [this, &context, &params](Dictionary::const_iterator::reference kvp){
            auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
            flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
            
            ElementOffset keyOffset = write(context, kvp.first);
            ElementOffset valueOffset = write(context, kvp.second);
            auto pairOffset = serialize::CreateDictionaryPair(builder, all_to_integral(keyOffset.second),
                    keyOffset.first, valueOffset.second, valueOffset.first);
            params.emplace_back(pairOffset);
        };
        std::for_each(value.begin(), value.end(), storeElement);
        auto vecOffset = builder.CreateVector(params);
        auto offset = serialize::CreateDictionary(builder, vecOffset);
        flatContext.add_offset(serialize::all_types::all_types_Dictionary, offset.Union());
    }
    
    flatbuffers::Offset<serialize::Container> SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const _Container& value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.get_builder();
        std::vector<flatbuffers::Offset<serialize::ContainerParam>> params;
        auto storeElement = [&builder, &params](Tuple::const_iterator::reference param){ //elements are stored to the buffer backward
            if(param.IsLong() || param.IsInt())
            {
                const auto& typedParam = static_cast<const core::TypedParam<int>&>(param);
                auto offset = serialize::CreateInt(builder, typedParam.Get<int>());
                params.emplace_back(serialize::CreateContainerParam(builder, serialize::all_types::all_types_Int, offset.Union()));
            }
            else if(param.IsCtypeS())
            {
                const auto& typedParam = static_cast<const core::TypedParam<char*>&>(param);
                auto strOffset = builder.CreateString(typedParam.Get<char*>());
                auto offset = serialize::CreateCTypeString(builder, strOffset);
                params.emplace_back(serialize::CreateContainerParam(builder, serialize::all_types::all_types_CTypeString, offset.Union()));
            }
            else if(param.IsString())
            {
                const auto& typedParam = static_cast<const core::TypedParam<std::string>&>(param);
                auto strOffset = builder.CreateString(typedParam.Get<std::string>());
                auto offset = serialize::CreateString(builder, strOffset);
                params.emplace_back(serialize::CreateContainerParam(builder, serialize::all_types::all_types_String, offset.Union()));
            }
            else if(param.IsBool())
            {
                const auto& typedParam = static_cast<const core::TypedParam<bool>&>(param);
                auto offset = serialize::CreateBool(builder, typedParam.Get<bool>());
                params.emplace_back(serialize::CreateContainerParam(builder, serialize::all_types::all_types_Bool, offset.Union()));
            }
            else if(param.IsDouble())
            {
                const auto& typedParam = static_cast<const core::TypedParam<double>&>(param);
                auto offset = serialize::CreateDouble(builder, typedParam.Get<double>());
                params.emplace_back(serialize::CreateContainerParam(builder, serialize::all_types::all_types_Double, offset.Union()));
            }
            else
                throw core::Exception(__CORE_SOURCE, "Non supported types");
        };
        std::for_each(value.rbegin(), value.rend(), storeElement);
        auto vecOffset = builder.CreateVector(params);
        return serialize::CreateContainer(builder, vecOffset);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::write(SerializeContext& context, const ObjectPtr& object) const
    {
        if(object->ob_type == &PyLong_Type)
        {
            write(context, Object<int>::from_python(object.get()));
        }
        else if(object->ob_type == &PyUnicode_Type)
        {
            write(context, Object<AsciiString>::from_python(object.get()).get_str().c_str());
        }
        else if(object->ob_type == &PyFloat_Type)
        {
            write(context, Object<double>::from_python(object.get()));
        }
        else if(object->ob_type == &PyByteArray_Type)
        {
            write(context, Object<std::string>::from_python(object.get()));
        }
        else if(object->ob_type == &PyBool_Type)
        {
            write(context, Object<bool>::from_python(object.get()));
        }
        else if(object->ob_type == &PyTuple_Type)
        {
            write(context, Object<sweetPy::Tuple>::from_python(object.get()));
        }
        else if(object->ob_type == &PyList_Type)
        {
            write(context, Object<sweetPy::List>::from_python(object.get()));
        }
        else if(object->ob_type == &PyDict_Type)
        {
            write(context, Object<sweetPy::Dictionary>::from_python(object.get()));
        }
        else
            throw core::Exception(__CORE_SOURCE, "Non supported type requested - %d", object->ob_type);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::start_read(SerializeContext& context, const SerializeContext::String& buffer)
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        m_it.reset(new _Context::const_iterator(flatContext.start_read(buffer)));
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::start_read(SerializeContext& context, char* buffer, std::size_t size)
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        static auto _deallocate = [](char*){};
        SerializeContext::String _buffer(std::make_pair(
                SerializeContext::_Buffer(buffer, _deallocate),
                size));
        m_it.reset(new _Context::const_iterator(flatContext.start_read(_buffer)));
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(int& value)
    {
        value = (*m_it)->get<serialize::Int>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(double& value)
    {
        value = (*m_it)->get<serialize::Double>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(bool& value)
    {
        value = (*m_it)->get<serialize::Bool>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(std::string& value)
    {
        if((*m_it)->get_type() == serialize::all_types_CTypeString)
            value = (*m_it)->get<serialize::CTypeString>().value()->str();
        else
            value = (*m_it)->get<serialize::String>().value()->str();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(Tuple& value)
    {
        auto& offsets = *(*m_it)->get<serialize::Container>().params();
        read(offsets, static_cast<_Container&>(value));
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(List& value)
    {
        auto& offsets = *(*m_it)->get<serialize::Container>().params();
        read(offsets, static_cast<_Container&>(value));
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(Dictionary& value)
    {
        auto& offsets = *(*m_it)->get<serialize::Dictionary>().params();
        for(const serialize::DictionaryPair* kvp: offsets)
        {
            ObjectPtr keyObject = read_dictionary_key(*kvp);
            ObjectPtr valueObject = read_dictionary_value(*kvp);
            value.add(keyObject, valueObject);
        }
        ++(*m_it);
    }
    
    ObjectPtr SweetPickleImpl<SerializeType::FlatBuffers>::read_dictionary_key(const serialize::DictionaryPair& pair)
    {
        if(pair.key_type() == serialize::integral_types::integral_types_Bool)
            return ObjectPtr(Object<bool>::to_python(pair.key_as_Bool()->value()), &Deleter::Owner);
        else if(pair.key_type() == serialize::integral_types::integral_types_Short ||
                pair.key_type() == serialize::integral_types::integral_types_Int)
            return ObjectPtr(Object<int>::to_python(pair.key_as_Int()->value()), &Deleter::Owner);
        else if(pair.key_type() == serialize::integral_types::integral_types_Double)
            return ObjectPtr(Object<double>::to_python(pair.key_as_Double()->value()), &Deleter::Owner);
        else if(pair.key_type() == serialize::integral_types::integral_types_CTypeString)
            return ObjectPtr(Object<AsciiString>::to_python(AsciiString(
                    pair.key_as_CTypeString()->value()->str())), &Deleter::Owner);
        else if(pair.key_type() == serialize::integral_types::integral_types_String)
            return ObjectPtr(Object<std::string>::to_python(std::string(
                    pair.key_as_String()->value()->str())), &Deleter::Owner);
        else
            throw core::Exception(__CORE_SOURCE, "Non supported type was provided - %d", pair.key_type());
    }
    
    ObjectPtr SweetPickleImpl<SerializeType::FlatBuffers>::read_dictionary_value(const serialize::DictionaryPair& pair)
    {
        if(pair.value_type() == serialize::all_types::all_types_Bool)
            return ObjectPtr(Object<bool>::to_python(pair.value_as_Bool()->value()), &Deleter::Owner);
        else if(pair.value_type() == serialize::all_types::all_types_Short ||
                pair.value_type() == serialize::all_types::all_types_Int)
            return ObjectPtr(Object<int>::to_python(pair.value_as_Int()->value()), &Deleter::Owner);
        else if(pair.value_type() == serialize::all_types::all_types_Double)
            return ObjectPtr(Object<double>::to_python(pair.value_as_Double()->value()), &Deleter::Owner);
        else if(pair.value_type() == serialize::all_types::all_types_CTypeString)
            return ObjectPtr(Object<AsciiString>::to_python(AsciiString(
                    pair.value_as_CTypeString()->value()->str())), &Deleter::Owner);
        else if(pair.value_type() == serialize::all_types::all_types_String)
            return ObjectPtr(Object<std::string>::to_python(std::string(
                    pair.value_as_String()->value()->str())), &Deleter::Owner);
        else if(pair.value_type() == serialize::all_types::all_types_List)
        {
            List list;
            auto offsets = pair.value_as_List()->value()->params();
            read(*offsets, static_cast<_Container&>(list));
            return ObjectPtr(Object<List>::to_python(list), &Deleter::Owner);
        }
        else if(pair.value_type() == serialize::all_types::all_types_Tuple)
        {
            Tuple tuple;
            auto offsets = pair.value_as_Tuple()->value()->params();
            read(*offsets, static_cast<_Container&>(tuple));
            return ObjectPtr(Object<Tuple>::to_python(tuple), &Deleter::Owner);
        }
        else
            throw core::Exception(__CORE_SOURCE, "Non supported type was provided - %d", pair.value_type());
            
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::read(const ContainerParams& params, _Container& value)
    {
        for(int idx = params.size() - 1; idx >= 0; idx--)
        {
           auto offset = params[idx];
            if(offset->param_type() == serialize::all_types::all_types_Bool)
                value.add_element(offset->param_as_Bool()->value());
            else if(offset->param_type() == serialize::all_types::all_types_Double)
                value.add_element(offset->param_as_Double()->value());
            else if(offset->param_type() == serialize::all_types::all_types_Short)
                value.add_element(offset->param_as_Short()->value());
            else if(offset->param_type() == serialize::all_types::all_types_Int)
                value.add_element(offset->param_as_Int()->value());
            else if(offset->param_type() == serialize::all_types::all_types_String)
                value.add_element(offset->param_as_String()->value()->str());
            else if(offset->param_type() == serialize::all_types::all_types_CTypeString)
                value.add_element(offset->param_as_CTypeString()->value()->str().c_str());
            else
                throw core::Exception(__CORE_SOURCE, "Non supported type was provided - %d", offset->param_type());
        }
    }
    
    serialize::integral_types
    SweetPickleImpl<SerializeType::FlatBuffers>::all_to_integral(const serialize::all_types allTypes)
    {
        switch(allTypes)
        {
            case serialize::all_types::all_types_Int:
                return serialize::integral_types::integral_types_Int;
            case serialize::all_types::all_types_Short:
                return serialize::integral_types::integral_types_Short;
            case serialize::all_types::all_types_Double:
                return serialize::integral_types::integral_types_Double;
            case serialize::all_types::all_types_Bool:
                return serialize::integral_types::integral_types_Bool;
            case serialize::all_types::all_types_String:
                return serialize::integral_types::integral_types_String;
            case serialize::all_types::all_types_CTypeString:
                return serialize::integral_types::integral_types_CTypeString;
            default:
                throw core::Exception(__CORE_SOURCE, "No conversion between all_types -{%d} to integral_types", allTypes);
        }
    }
    
    serialize::all_types SweetPickleImpl<SerializeType::FlatBuffers>::get_type() const
    {
        return (*m_it)->get_type();
    }
    
    class SweetPickleCreator
    {
    public:
        virtual ~SweetPickleCreator() {}
        virtual std::unique_ptr<SweetPickle> create() = 0;
        virtual std::unique_ptr<SerializeContext> create_context() = 0;
    };

    template<typename ConcreteSweetPickle, typename ConcreteSerializeContext>
    class ConcreteSweetPickleCreator : public SweetPickleCreator
    {
    public:
        virtual ~ConcreteSweetPickleCreator() {}
        std::unique_ptr<SweetPickle> create() override {
            return std::unique_ptr<SweetPickle>(new ConcreteSweetPickle());
        }
    
        std::unique_ptr<SerializeContext> create_context() override {
            return std::unique_ptr<SerializeContext>(new ConcreteSerializeContext());
        }
    };

    class SweetPickleFactory
    {
    public:
        static const SweetPickleFactory& instance();
        ~SweetPickleFactory() {}
        std::unique_ptr<SweetPickle> create(SerializeType type) const;
        std::unique_ptr<SerializeContext> create_context(SerializeType type) const;
        
    private:
        SweetPickleFactory();
    private:
        std::unordered_map<SerializeType, std::unique_ptr<SweetPickleCreator>, SerializeType::Hash> m_serializors;
    };

}

