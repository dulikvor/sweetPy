#pragma once

namespace pycppconnTest {

    class CPythonClassTestSubject {
    public:
        CPythonClassTestSubject(int &valueInt) : m_byValueInt(valueInt) {}
        ~CPythonClassTestSubject(){ m_instanceDestroyed = true; }
        virtual int GetValue(){ return m_byValueInt;}

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
    };
}

