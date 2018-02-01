#pragma once

#include <functional>
#include <Python.h>
#include "Lock.h"
#include "Exception.h"

namespace pycppconn{
#define CPYTHON_VERIFY(expression, reason) do{ if(!expression) throw CPythonException(PyExc_StandardError, SOURCE, reason); }while(0)
}