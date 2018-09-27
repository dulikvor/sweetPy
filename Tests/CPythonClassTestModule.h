#pragma once

#include <Python.h>
#include <cstring>
#include <string>
#include <memory>
#include <vector>

namespace sweetPyTest {

    template<typename T>
    class GenerateRefTypes
    {
    public:
        GenerateRefTypes(){m_values.reserve(1000);}
        T& operator()(typename std::remove_const<T>::type const& value)
        {
            m_values.push_back(value);
            return m_values.back();
        }
    private:
        std::vector<typename std::remove_const<T>::type> m_values;
    };

    template<>
    class GenerateRefTypes<char*>
    {
    public:
        GenerateRefTypes(){m_values.reserve(1000);}
        ~GenerateRefTypes()
        {
            for(auto& ptr : m_values)
                delete[] ptr;
        }
        char*& operator()(char* value)
        {
            m_values.push_back(nullptr);
            size_t size = strlen(value);
            m_values.back() = new char[size + 1];
            memcpy(m_values.back(), value, size);
            m_values.back()[size] = '\0';
            return m_values.back();
        }
    private:
        std::vector<char*> m_values;
    };

    int CheckIntegralIntType(int value){ return value + 1; }
    int& CheckIntegralIntType(int& value){ return ++value; }
    const int& CheckIntegralIntType(const int& value){ static std::vector<int> values(100); values.push_back(value); return values.back(); }
    void CheckIntegralIntType(int&& value){ static int i = 0; std::swap(value, i);  }

    std::string CheckIntegralStringType(std::string value){ return value + " world"; }
    std::string& CheckIntegralStringType(std::string& value){ return value = "hello to all"; }
    const std::string& CheckIntegralStringType(const std::string& value){ static std::vector<std::string> values(100); values.push_back(value); return values.back(); }
    void CheckIntegralStringType(std::string&& value) { static std::string str = "great!"; str = std::move(value); }
    char(&CheckIntegralCharArrayType(char(&value)[100]))[100]{ return value; }

    void CheckIntegralCTypeStringType(char* value){ *value = 'l'; }
    void CheckIntegralConstCTypeStringType(const char* value){}

    PyObject* CheckIntegralPyObjectType(PyObject* value){ return value; }

    enum Python
    {
        Good = 1,
        Bad = 3
    };

    int globalFunction(int i){return i;}

    class TestSubjectC{
    public:
        TestSubjectC(const TestSubjectC&) = delete;
        TestSubjectC& operator=(const TestSubjectC&) = delete;
        TestSubjectC(const TestSubjectC&&) = delete;
        TestSubjectC& operator=(const TestSubjectC&&) = delete;
        static TestSubjectC& Instance(){
            static TestSubjectC c{};
            return c;
        }
        void Inc(){i++;}

    public:
        int i;

    private:
        TestSubjectC(){}

    };

    class TestSubjectB{
    public:
        TestSubjectB():m_value(0), m_str("Hello World"){}
        void IncValue(){m_value++;}
        int GetValue() const { return m_value; }
        int Foo(int i){ return i; }
        int Foo(int i, int y){return i+y;}

    public:
        int m_value;
        std::string m_str;
    };

    class TestSubjectAAbastract
    {
    public:
        TestSubjectAAbastract():m_value{}{}
        virtual void IncBaseValue(){m_value++;}
        virtual int GetBaseValue(){return m_value;}
    private:
        int m_value;
    };

    class TestSubjectA : public TestSubjectAAbastract{
    public:
        TestSubjectA(int valueInt) : m_byValueInt(valueInt), m_enumValue(Python::Good) {}
        ~TestSubjectA(){ m_instanceDestroyed = true; }
        virtual int GetValue(){ return m_byValueInt;}
        const std::string& GetStr() const{return m_str;}
        void SetXpireValue(std::string&& str){m_str = str;}
        std::string SetString(const std::string& str){
            return m_str = str + " Temp";
        }

        static std::unique_ptr<TestSubjectA> GetUniqueMe(){
            static int i = 5;
            return std::unique_ptr<TestSubjectA>(new TestSubjectA(i));
        }

        TestSubjectA& GetMe(){
            return *this;
        }

        void SetPython(Python enumValue){
            m_enumValue = enumValue;
        }

        const TestSubjectB& GetB() const{
            return m_b;
        }

        TestSubjectB GetBByValue() const{
            return m_b;
        }

        void IncBByRef(std::unique_ptr<TestSubjectB>& b) const
        {
            b->IncValue();
        }

        void IncB(std::unique_ptr<TestSubjectB> b) const
        {
            b->IncValue();
        }

        std::unique_ptr<TestSubjectB> GetBNonCopyConstructable() const{
            return std::unique_ptr<TestSubjectB>(new TestSubjectB(m_b));
        }

        std::vector<int> FromStrVectorToIntVector(const std::vector<std::string>& strVector)
        {
            std::vector<int> intVector;
            for(const std::string& str : strVector)
                intVector.push_back(str.length());
            return intVector;
        }

        static void BMutator(TestSubjectB& obj){
            obj.IncValue();
        }

        static void Setter() {
            m_valid = true;
        }

        static bool Getter() {
            return m_valid;
        }
        static bool GetInstancDestroyed(){
            return m_instanceDestroyed;
        }

    public:
        static bool m_valid;
        static bool m_instanceDestroyed;
        int m_byValueInt;
        TestSubjectB m_b;
        std::string m_str;
        Python m_enumValue;
        const char* m_ctypeStr;
    };

}

