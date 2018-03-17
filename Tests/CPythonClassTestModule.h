#pragma once

namespace pycppconnTest {

    class CPythonClassTestSubject {
    public:
        CPythonClassTestSubject(int &valueInt) : m_byValueInt(valueInt) {}

        static void Setter() {
            m_valid = true;
        }

        static bool Getter() {
            return m_valid;
        }

    public:
        static bool m_valid;
        int m_byValueInt;
    };
}

