#pragma once
namespace sweetPy{
    struct Hash
    {
    private:
        template<typename T>
        struct _Hash{};
        
    public:
        template<typename T>
        static std::size_t generate_hash_code()
        {
            return typeid(_Hash<T>).hash_code();
        }
    };
}
