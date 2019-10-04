#include "Detail/MetaClass.h"

namespace sweetPy {
    
    MetaClass MetaClass::m_commonMetaType("common_meta_type", "");
    MetaClass* commonMetaTypePtr = &MetaClass::get_common_meta_type();
}

