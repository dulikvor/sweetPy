#include "Utility/Serialize.h"


namespace sweetPy{
    
    const SweetPickleFactory& SweetPickleFactory::instance()
    {
        static SweetPickleFactory instance;
        return instance;
    }

    SweetPickleFactory::SweetPickleFactory()
    {
        m_serializors.emplace(SerializeType::FlatBuffers,
                           std::unique_ptr<ConcreteSweetPickleCreator<SweetPickleImpl<SerializeType::FlatBuffers>,
                              ConcreteSerializeContext<SerializeType::FlatBuffers>>>(
                                   new ConcreteSweetPickleCreator<SweetPickleImpl<SerializeType::FlatBuffers>,
                                                                   ConcreteSerializeContext<SerializeType::FlatBuffers>>()));
    }

    std::unique_ptr<SweetPickle> SweetPickleFactory::create(SerializeType type) const
    {
        auto it = m_serializors.find(type);
        if (it == m_serializors.end())
            throw core::Exception(__CORE_SOURCE, "Requested serialize type - %s is not supported", type.ToString().c_str());
        return it->second->create();
    }
    
    std::unique_ptr<SerializeContext> SweetPickleFactory::create_context(SerializeType type) const
    {
        auto it = m_serializors.find(type);
        if (it == m_serializors.end())
            throw core::Exception(__CORE_SOURCE, "Requested serialize type - %s is not supported", type.ToString().c_str());
        return it->second->create_context();
    }
}
