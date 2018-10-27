#include "Lock.h"

namespace sweetPy{
    thread_local PyThreadState* Yield::m_save = nullptr;
}