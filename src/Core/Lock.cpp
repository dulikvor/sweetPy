#include "Lock.h"

namespace sweetPy{
    thread_local bool Yield::m_alreadyYield = false;
}