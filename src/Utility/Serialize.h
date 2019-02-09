#pragma once

#include <Python.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include "../Core/Deleter.h"
#include "../CPythonObject.h"
#include "../Types/Tuple.h"
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
        virtual void Write(SerializeContext& context, const object_ptr& object) const = 0;
        virtual void Write(SerializeContext& context, int value) const = 0;
        virtual void Write(SerializeContext& context, double value) const = 0;
        virtual void Write(SerializeContext& context, const char* value) const = 0;
        virtual void Write(SerializeContext& context, const std::string& value) const = 0;
        virtual void Write(SerializeContext& context, bool value) const = 0;
        virtual void Write(SerializeContext& context, const Tuple& value) const = 0;
        virtual void StartRead(SerializeContext& context, const SerializeContext::String& buffer) = 0;
        virtual void Read(int& value) = 0;
        virtual void Read(double& value) = 0;
        virtual void Read(bool& value) = 0;
        virtual void Read(std::string& value) = 0;
        virtual void Read(Tuple& value) = 0;
        virtual serialize::all_types GetType() const = 0;
    };
    
    template<SerializeType::Enumeration>
    class SweetPickleImpl : public SweetPickle
    {
    public:
        virtual ~SweetPickleImpl(){}
        void Write(SerializeContext& context, const object_ptr& object) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Write(SerializeContext& context, int value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Write(SerializeContext& context, double value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Write(SerializeContext& context, const char* value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Write(SerializeContext& context, const std::string& value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
        
        void Write(SerializeContext& context, bool value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Write(SerializeContext& context, const Tuple& value) const override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
    
        void StartRead(SerializeContext& context, const SerializeContext::String& buffer) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Read(int& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Read(double& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Read(bool& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Read(std::string& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        void Read(Tuple& value) override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    
        serialize::all_types GetType() override
        {
            throw core::Exception(__CORE_SOURCE, "Unsupported method");
        }
    };
    
    template<>
    class SweetPickleImpl<SerializeType::FlatBuffers> : public SweetPickle
    {
    public:
        virtual ~SweetPickleImpl(){}
        void Write(SerializeContext& context, const object_ptr& object) const override;
        void Write(SerializeContext& context, int value) const override;
        void Write(SerializeContext& context, double value) const override;
        void Write(SerializeContext& context, const char* value) const override;
        void Write(SerializeContext& context, const std::string& value) const override;
        void Write(SerializeContext& context, bool value) const override;
        void Write(SerializeContext& context, const Tuple& value) const override;
        void StartRead(SerializeContext& context, const SerializeContext::String& buffer) override;
        void Read(int& value) override;
        void Read(double& value) override;
        void Read(bool& value) override;
        void Read(std::string& value) override;
        void Read(Tuple& value) override;
        serialize::all_types GetType() const override;
        
    private:
        typedef ConcreteSerializeContext<SerializeType::FlatBuffers> _Context;
        std::unique_ptr<_Context::const_iterator> m_it;
    };
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, int value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        auto offset = serialize::CreateInt(builder, value);
        flatContext.AddOffset(serialize::all_types::Int, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, double value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        auto offset = serialize::CreateDouble(builder, value);
        flatContext.AddOffset(serialize::all_types::Double, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, const char* value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        auto strOffset = builder.CreateString(value);
        auto offset = serialize::CreateString(builder, strOffset);
        flatContext.AddOffset(serialize::all_types::String, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, const std::string& value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        auto strOffset = builder.CreateString(value);
        auto offset = serialize::CreateString(builder, strOffset);
        flatContext.AddOffset(serialize::all_types::String, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, bool value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        auto offset = serialize::CreateBool(builder, value);
        flatContext.AddOffset(serialize::all_types::Bool, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, const Tuple& value) const
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        flatbuffers::FlatBufferBuilder& builder = flatContext.GetBuilder();
        std::vector<flatbuffers::Offset<serialize::TupleParam>> params;
        auto storeElement = [&builder, &params](Tuple::const_iterator::reference param){ //elements are stored to the buffer backward
            if(param.IsLong() || param.IsInt())
            {
                const auto& typedParam = static_cast<const core::TypedParam<int>&>(param);
                auto offset = serialize::CreateInt(builder, typedParam.Get<int>());
                params.emplace_back(serialize::CreateTupleParam(builder, serialize::integral_types::Int, offset.Union()));
            }
            else if(param.IsCtypeS())
            {
                const auto& typedParam = static_cast<const core::TypedParam<char*>&>(param);
                auto strOffset = builder.CreateString(typedParam.Get<char*>());
                auto offset = serialize::CreateString(builder, strOffset);
                params.emplace_back(serialize::CreateTupleParam(builder, serialize::integral_types::String, offset.Union()));
            }
            else if(param.IsString())
            {
                const auto& typedParam = static_cast<const core::TypedParam<std::string>&>(param);
                auto strOffset = builder.CreateString(typedParam.Get<std::string>());
                auto offset = serialize::CreateString(builder, strOffset);
                params.emplace_back(serialize::CreateTupleParam(builder, serialize::integral_types::String, offset.Union()));
            }
            else if(param.IsBool())
            {
                const auto& typedParam = static_cast<const core::TypedParam<bool>&>(param);
                auto offset = serialize::CreateBool(builder, typedParam.Get<bool>());
                params.emplace_back(serialize::CreateTupleParam(builder, serialize::integral_types::Bool, offset.Union()));
            }
            else if(param.IsDouble())
            {
                const auto& typedParam = static_cast<const core::TypedParam<double>&>(param);
                auto offset = serialize::CreateDouble(builder, typedParam.Get<double>());
                params.emplace_back(serialize::CreateTupleParam(builder, serialize::integral_types::Double, offset.Union()));
            }
            else
                throw core::Exception(__CORE_SOURCE, "Non supported types");
        };
        std::for_each(value.rbegin(), value.rend(), storeElement);
        auto vecOffset = builder.CreateVector(params);
        auto offset = serialize::CreateTuple(builder, vecOffset);
        flatContext.AddOffset(serialize::all_types::Tuple, offset.Union());
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Write(SerializeContext& context, const object_ptr& object) const
    {
        if(object->ob_type == &PyLong_Type)
        {
            Write(context, Object<int>::FromPython(object.get()));
        }
        else if(object->ob_type == &PyUnicode_Type)
        {
            Write(context, Object<std::string>::FromPython(object.get()));
        }
        else if(object->ob_type == &PyFloat_Type)
        {
            Write(context, Object<double>::FromPython(object.get()));
        }
        else if(object->ob_type == &PyByteArray_Type)
        {
            Write(context, Object<std::string>::FromPython(object.get()));
        }
        else if(object->ob_type == &PyBool_Type)
        {
            Write(context, Object<bool>::FromPython(object.get()));
        }
        else if(object->ob_type == &PyTuple_Type)
        {
            Write(context, Object<sweetPy::Tuple>::FromPython(object.get()));
        }
        else
            throw core::Exception(__CORE_SOURCE, "Non supported type requested - %d", object->ob_type);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::StartRead(SerializeContext& context, const SerializeContext::String& buffer)
    {
        auto& flatContext = static_cast<ConcreteSerializeContext<SerializeType::FlatBuffers>&>(context);
        m_it.reset(new _Context::const_iterator(flatContext.StartRead(buffer)));
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Read(int& value)
    {
        value = (*m_it)->Get<serialize::Int>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Read(double& value)
    {
        value = (*m_it)->Get<serialize::Double>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Read(bool& value)
    {
        value = (*m_it)->Get<serialize::Bool>().value();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Read(std::string& value)
    {
        value = (*m_it)->Get<serialize::String>().value()->str();
        ++(*m_it);
    }
    
    void SweetPickleImpl<SerializeType::FlatBuffers>::Read(Tuple& value)
    {
        auto& offsets = *(*m_it)->Get<serialize::Tuple>().params();
        for(int idx = offsets.size() - 1; idx >= 0; idx--)
        {
           auto offset = offsets[idx];
            if(offset->param_type() == serialize::integral_types::Bool)
                value.AddElement(offset->param_as_Bool()->value());
            else if(offset->param_type() == serialize::integral_types::Double)
                value.AddElement(offset->param_as_Double()->value());
            else if(offset->param_type() == serialize::integral_types::Short)
                value.AddElement(offset->param_as_Short()->value());
            else if(offset->param_type() == serialize::integral_types::Int)
                value.AddElement(offset->param_as_Int()->value());
            else if(offset->param_type() == serialize::integral_types::String)
                value.AddElement(offset->param_as_String()->value()->str());
        }
        ++(*m_it);
    }
    
    serialize::all_types SweetPickleImpl<SerializeType::FlatBuffers>::GetType() const
    {
        return (*m_it)->GetType();
    }
    
    class SweetPickleCreator
    {
    public:
        virtual ~SweetPickleCreator() {}
        virtual std::unique_ptr<SweetPickle> Create() = 0;
        virtual std::unique_ptr<SerializeContext> CreateContext() = 0;
    };

    template<typename ConcreteSweetPickle, typename ConcreteSerializeContext>
    class ConcreteSweetPickleCreator : public SweetPickleCreator
    {
    public:
        virtual ~ConcreteSweetPickleCreator() {}
        std::unique_ptr<SweetPickle> Create() override {
            return std::unique_ptr<SweetPickle>(new ConcreteSweetPickle());
        }
    
        std::unique_ptr<SerializeContext> CreateContext() override {
            return std::unique_ptr<SerializeContext>(new ConcreteSerializeContext());
        }
    };

    class SweetPickleFactory
    {
    public:
        static const SweetPickleFactory& Instance();
        ~SweetPickleFactory() {}
        std::unique_ptr<SweetPickle> Create(SerializeType type) const;
        std::unique_ptr<SerializeContext> CreateContext(SerializeType type) const;
        
    private:
        SweetPickleFactory();
    private:
        std::unordered_map<SerializeType, std::unique_ptr<SweetPickleCreator>, SerializeType::Hash> m_serializors;
    };

}

