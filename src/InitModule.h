#pragma once

#include <string>
#include <type_traits>
#include <memory>
#include <vector>
#include <Python.h>
#include "Common.h"
#include "CPyModuleContainer.h"
#include "CPythonMetaClass.h"
#include "CPythonRef.h"
#include "CPythonModule.h"

#define INIT_MODULE(name, doc) \
void InitializeModule(pycppconn::CPythonModule& module); \
PyMODINIT_FUNC init##name() { \
    auto module = std::make_shared<pycppconn::CPythonModule>(#name, doc); \
    pycppconn::CPyModuleContainer::Instance().AddModule(#name, module); \
    pycppconn::CPythonMetaClass<>::InitStaticType(); \
    pycppconn::CPythonRef<>::InitStaticType(); \
    InitializeModule(*module); \
} \
void InitializeModule(pycppconn::CPythonModule& module)
