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

## Constructor:
It is possible to explicitly define a constructor for the exported user type.

This can be done by using a special constructor method accessiable via your newly created CPythonClass object, wrapping your to be exported UserType:
```c++
myType.AddConstructor<>();
```
If more than one Constructor exists, its up to the user to select the one it wishes to export, as of now, only a single constructor is supported.

The user may select his constructor of choice by explicitly initialising the templated constructor method with a matching arguments types depicting its constructor of choice:
```c++
Class MyType
{
public:
	MyType(int, std::string){...}
...
myType.AddConstructor<int, std::string>();
```

The constructor must be made public and accessible, as if a type trait of ```std::is_constructiable<…>``` was being used.

If the constructed method is being initialised with no types at all, default constructor will be used.
```c++
myType.AddConstructor<>();
```

If the user didn’t declared any specific constructor, it is as if - default constructor was declared.

## Methods:

Publicly accessible methods can be exported.
This can be done by using a specific method via the CPythonClass object, and by providing the method - python's name, description and the method virtual address.
```c++
Class MyType
{
public:
	int foo(const int&){...}
...
myType.AddMethod("foo", "foo method", &MyType::foo);
```
The method prototype, including its - parameters and return type are deducted automatically, only the method virtual address is required, both virtual and non virtual methods are supported this way:

Few restrictions:
If an overload set exists, its up to the user to explicitly choose the overloaded version he wishes to export.
```c++
Class MyType
{
public:
	int foo(const int&){...}
	float foo(std::string&, float){...}
...
myType.AddMethod("foo", "foo method", static_cast<float(MyType::*)(std::string&, float)>(&MyType::foo));
```
Every overload version will need to have a different python’s name.
```c++
myType.AddMethod("foo_1", "foo method", static_cast<int(MyType::*)(const int&)>(&MyType::foo));
myType.AddMethod("foo_2", "foo method", static_cast<float(MyType::*)(std::string&, float)>(&MyType::foo));
```
Templated function will need to be explicitly initialised with the version wishes to be exported.
```c++
Class MyType
{
public:
	template<typename T> int foo(const T&){...}
...
myType.AddMethod("foo", "foo method", &MyType::foo<int>);
```

## Members:
Publicly accessible members can be exported.
This can be done by using a specific method via the CPythonClass Object and by providing the member python’s name, virtual address, and description. 
```c++
Class MyType
{
public:
	int m_val;
...
myType.AddMember("val", &MyType::m_val, "val member");
```
all deductions, including the member offset with in the instance image and its type, will be done automatically.

All members are considered read/write accessible.
