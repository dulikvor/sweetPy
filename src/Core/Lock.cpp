#include "Core/Lock.h"

namespace sweetPy{
    thread_local bool GilRelease::m_alreadyReleased = false;
}