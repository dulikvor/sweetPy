#pragma once

#include <Python.h>
#include <string>
#include <type_traits>
#include <memory>
#include <vector>
#include "Core/Traits.h"
#include "src/Detail/TypesContainer.h"
#include "src/Detail/MetaClass.h"
#include "src/Detail/ReferenceType.h"
#include "Module.h"

#define INIT_MODULE(name, doc) \
void initialize_module(sweetPy::Module& module); \
PyMODINIT_FUNC PyInit_##name() { \
    Module module(#name, doc); \
    sweetPy::MetaClass::get_common_meta_type().finalize(); \
    initialize_module(module); \
    module.finalize();\
    return module.get_module(); \
} \
void initialize_module(sweetPy::Module& module)
