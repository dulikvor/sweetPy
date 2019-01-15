#pragma once

#include <Python.h>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include "core/Assert.h"
#include "Core/Deleter.h"
#include "Types/DateTime.h"
#include "Types/TimeDelta.h"
#include "Types/Tuple.h"
#include "Types/AsciiString.h"
#include "CPythonObject.h"

namespace sweetPyTest {

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
        const std::string& GetStr() const { return m_str; }
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


    template<typename T>
    class GenerateRefTypes
    {
    public:
        GenerateRefTypes(){m_values.reserve(1000);}
        template<typename X = T>
        typename std::enable_if<std::is_copy_constructible<X>::value, X&>::type operator()(typename std::remove_const<X>::type const& value)
        {
            m_values.push_back(value);
            return m_values.back();
        }
    
        template<typename X = T, typename  = typename std::enable_if<std::is_same<typename std::remove_const<X>::type, sweetPy::object_ptr>::value>::type>
        const sweetPy::object_ptr& operator()(sweetPy::object_ptr const& value)
        {
            Py_XINCREF(value.get());
            m_values.emplace_back(value.get(), &sweetPy::Deleter::Owner);
            return m_values.back();
        }
        
        void Clear()
        {
            m_values.clear();
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
    
    template<>
    class GenerateRefTypes<const char*>
    {
    public:
        GenerateRefTypes(){m_values.reserve(1000);}
        ~GenerateRefTypes()
        {
            for(auto& ptr : m_values)
                delete[] ptr;
        }
        char const *& operator()(const char*& value)
        {
            m_values.push_back(nullptr);
            size_t size = strlen(value);
            m_values.back() = new char[size + 1];
            memcpy(m_values.back(), value, size);
            m_values.back()[size] = '\0';
            return const_cast<const char*&>(m_values.back());
        }
    private:
        std::vector<char*> m_values;
    };

    int CheckIntegralIntType(int value){ return value + 1; }
    int& CheckIntegralIntType(int& value){ return ++value; }
    const int& CheckIntegralIntType(const int& value){ static std::vector<int> values(100); values.push_back(value); return values.back(); }
    void CheckIntegralIntType(int&& value){ static int i = 0; std::swap(value, i);  }

    double CheckIntegralDoubleType(double value){ return value + 1; }
    double& CheckIntegralDoubleType(double& value){ return ++value; }
    const double& CheckIntegralDoubleType(const double& value){ static std::vector<double> values(100); values.push_back(value); return values.back(); }
    void CheckIntegralDoubleType(double&& value){ static double i = 0; std::swap(value, i);  }

    std::string CheckIntegralStringType(std::string value){ return value + " world"; }
    std::string& CheckIntegralStringType(std::string& value){ return value = "hello to all"; }
    const std::string& CheckIntegralStringType(const std::string& value){ static std::vector<std::string> values(100); values.push_back(value); return values.back(); }
    void CheckIntegralStringType(std::string&& value) { static std::string str = "great!"; str = std::move(value); }
    char(&CheckIntegralCharArrayType(char(&value)[100]))[100]{ return value; }

    void CheckIntegralCTypeStringType(char* value){ *value = 'l'; }
    const char* CheckIntegralConstCTypeStringType(const char* value){ static std::string str; str = value; return str.c_str(); }
    const char*& CheckIntegralConstRefCTypeStrType(const char*& value)
    {
        static std::vector<char*> values; //Known leakage.
        values.reserve(1000);
        size_t size = strlen(value);
        values.push_back(nullptr);
        values.back() = new char[size + 1];
        memcpy(values.back(), value, size);
        values.back()[size] = '\0';
        return const_cast<const char*&>(values.back());
    }

    sweetPy::DateTime CheckDateTimeType(sweetPy::DateTime value)
    {
        sweetPy::DateTime newValue(1, 2, 3, 4, 5);
        return newValue;
    }
    const sweetPy::DateTime& CheckConstRefDateTimeType(const sweetPy::DateTime& value)
    {
        static std::vector<sweetPy::DateTime> values;
        values.reserve(100);
        values.push_back(value);
        return values.back();
    }

    sweetPy::TimeDelta CheckTimeDeltaType(sweetPy::TimeDelta value)
    {
        sweetPy::TimeDelta newValue(3, 4, 5);
        return newValue;
    }
    const sweetPy::TimeDelta& CheckConstRefTimeDeltaType(const sweetPy::TimeDelta& value)
    {
        static std::vector<sweetPy::TimeDelta> values;
        values.reserve(100);
        values.push_back(value);
        return values.back();
    }

    sweetPy::Tuple CheckTuleType(sweetPy::Tuple value)
    {
        sweetPy::Tuple newValue;
        newValue.AddElement(0, 1);
        newValue.AddElement(1, 2.5);
        newValue.AddElement(2, "Goodbye");
        newValue.AddElement(3, std::string("World"));
        newValue.AddElement(4, true);
        return newValue;
    }

    const sweetPy::Tuple& CheckConstRefTupleType(const sweetPy::Tuple& value)
    {
        static std::vector<sweetPy::Tuple> values;
        values.reserve(100);
        values.push_back(value);
        return values.back();
    }
    
    sweetPy::AsciiString CheckAsciiStringType(sweetPy::AsciiString value)
    {
        sweetPy::AsciiString newValue("Babylon 5 Rulezzzzz!");
        return newValue;
    }
    
    const sweetPy::AsciiString& CheckConstRefAsciiStringType(const sweetPy::AsciiString& value)
    {
        static std::vector<sweetPy::AsciiString> values;
        values.reserve(100);
        values.push_back(value);
        return values.back();
    }
    
    sweetPy::object_ptr CheckObjectPtrType(sweetPy::object_ptr value)
    {
        CPYTHON_VERIFY_EXC(Py_TYPE(value.get()) == &PyLong_Type);
        int newInteger = sweetPy::Object<int>::FromPython(value.get()) + 1;
        return sweetPy::object_ptr(sweetPy::Object<int>::ToPython(newInteger), &sweetPy::Deleter::Owner);
    }
    
    const sweetPy::object_ptr& CheckConstRefObjectPtrType(const sweetPy::object_ptr& value)
    {
        CPYTHON_VERIFY_EXC(Py_TYPE(value.get()) == &PyLong_Type);
        int newInteger = sweetPy::Object<int>::FromPython(value.get()) + 1;
        
        static std::vector<sweetPy::object_ptr> values;
        values.reserve(100);
    
        sweetPy::object_ptr newValue(sweetPy::Object<int>::ToPython(newInteger), &sweetPy::Deleter::Borrow); //Python will not be there when the unique_ptr will try to deallocate the memory.
        values.emplace_back(std::move(newValue));
        return static_cast<const sweetPy::object_ptr&>(values.back());
    }
    
    sweetPy::Tuple GenerateNativeElementTuple()
    {
        sweetPy::Tuple tuple;
        static TestSubjectB testSubjectB;
        tuple.AddElement(0, testSubjectB, [](void const * const ptr) -> PyObject*{
            const TestSubjectB& value = *reinterpret_cast<const TestSubjectB*>(ptr);
            return sweetPy::Object<TestSubjectB>::ToPython(value);
        });
        return tuple;
    }
    
    PyObject* CheckIntegralPyObjectType(PyObject* value){ return value; }
}

