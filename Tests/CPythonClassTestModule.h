#pragma once

class CPythonClassTestSubject{
public:
    CPythonClassTestSubject(){}
    static void Setter(){
        m_valid = true;
    }
    static bool Getter(){
        return m_valid;
    }

public:
    static bool m_valid;
};

