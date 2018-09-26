#pragma once

#include <Python.h>
#include <string>
#include <type_traits>
#include <memory>
#include <vector>
#include "Core/Traits.h"
#include "CPyModuleContainer.h"
#include "CPythonMetaClass.h"
#include "CPythonRef.h"
#include "CPythonModule.h"

#define INIT_MODULE(name, doc) \
void InitializeModule(sweetPy::CPythonModule& module); \
PyMODINIT_FUNC PyInit_##name() { \
    sweetPy::CPyModuleContainer::Instance().RegisterContainer(); \
    auto module = std::make_shared<sweetPy::CPythonModule>(#name, doc); \
    sweetPy::CPyModuleContainer::Instance().AddModule(#name, module); \
    sweetPy::CPythonMetaClass::InitStaticType(); \
    sweetPy::CPythonRef<>::InitStaticType(); \
    InitializeModule(*module); \
    module->Finalize();\
    return module->GetModule(); \
} \
void InitializeModule(sweetPy::CPythonModule& module)
