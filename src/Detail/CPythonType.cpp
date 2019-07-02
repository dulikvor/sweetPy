#include "src/Detail/CPythonType.h"

namespace sweetPy {

    PyGC_Head CPythonType::m_nextStub{};
    PyGC_Head CPythonType::m_prevStub{};
    PyMemberDef CPythonType::MembersDefs::m_sentinal = {NULL};
}
