# User Types:
sweetPy provides a mean to export C++ user types with ease in an object oriented manner.

The user may export a variety of properties, implementing the state and logic of its user type.

Nothing is done automatically, and it’s up to the user to decide which part of its user type and its different properties and logic it wishes to export.

## Defining a new user type:
In order to define a new user type, the user will need to introduce the definition of its user type into the translation unit.

```c++
#include "MyType.h"
```
The way to define a new python user type is done by using the CPythonClass and initializing its template declaration with your selected user type, you will also need to provide the new type's name and documentation.

```c++
#include "sweetPy/CPythonClass.h
...
sweetPy::CPythonClass<MyType> myType(module, "MyType", "MyType description.");
```

The name will be your python type’s name, and the way to access it via the module.
```python
from MyModule import MyType
```
