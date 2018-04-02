#pragma once

#include <string>

namespace pycppconnTest {

    class TestSubjectB{
    public:
        TestSubjectB():m_value(0){}
        void IncValue(){m_value++;}
        int GetValue() const { return m_value; }

    private:
        int m_value;
    };

    class TestSubjectA {
    public:
        TestSubjectA(int &valueInt) : m_byValueInt(valueInt) {}
        ~TestSubjectA(){ m_instanceDestroyed = true; }
        virtual int GetValue(){ return m_byValueInt;}
        std::string SetString(const std::string& str){
            return m_str = str + " Temp";
        }
        TestSubjectB& GetB(){
            return m_b;
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
    };
}

