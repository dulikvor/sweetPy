#pragma once

#include <Python.h>
#include <string>
#include <type_traits>
#include <memory>
#include <vector>
#include "Core/Traits.h"
#include "Detail/TypesContainer.h"
#include "Detail/MetaClass.h"
#include "Detail/ReferenceType.h"
#include "Module.h"

#define INIT_MODULE(name, doc) \
void initialize_module(sweetPy::Module& module); \
PyMODINIT_FUNC PyInit_##name() { \
    sweetPy::Module module(#name, doc); \
    sweetPy::MetaClass::get_common_meta_type().finalize(); \
    initialize_module(module); \
    module.finalize();\
    return module.get_module(); \
} \
void initialize_module(sweetPy::Module& module)
