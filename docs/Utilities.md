# Utilities:

## Serialisation:
sweetPy provides serialise/deserialise capabilities based upon google’s flatbuffers package.

The interface is available solely in C++ and meant for:

* Serialising C++ selected integral and standard types along with sweetPy selected user types.
* Serialising python’s c-api PyObjects directly into the same stream.
* Deserialising the data into the same native types.

### Serialize:
In order to begin just include sweetPy header:
```c++
#include “sweetPy.h"
```
In order to serialise two user types instances are required:

**Serialisation context** - which will hold and manage our current to be sent buffer.

**SweetPickle** - provides the mean to write (Serialise) provided instances to a selected context.

In order to create the required two instances a special Factory exists:
```c++
auto& factory = sweetPy::SweetPickleFactory::Instance();
auto context = factory.CreateContext(sweetPy::SerializeType::FlatBuffers);
auto sweetPickle = factory.Create(sweetPy::SerializeType::FlatBuffers);
```
The factory different methods receives a unique enum depicting our method of serialisation, as of now only google’s flatbuffers package is supported.

SweetPickle instance provides a simple write method, which receives both the current context instance and to be serialised instance:
```c++
sweetPickle->Write(*contextSend, 5);
```

As of now the list of supported types is narrowed to:

| Group           | Types |
| -------------   | ------------- |
| C++ Integral + POD types    | int, short, bool, double, const char*  |
| C++ standard types            | std::string  |
| sweetPy types  | Tuple, List |

any type of C++ value category expressions is supported when provided the to be serialize instances.

### Retrieving the serialisation buffer:
once done, in order to retrieve the context underline buffer for future usage, 
the user needs to invoke Finish upon the context instance.
```c++
sweetPy::SerializeContext::String buffer = contextSend->Finish(false);
```
the user can decide either to **deep copy** the context's buffer **(boolean value == false)**, 
or to have direct access to it **(boolean value == true)**.

The buffer is returned in the form of - **c-type string**, pointing to the data, and its **size**.
if deep copy was chosen, the buffer will deallocate completely (raii is enforced) once the scope of the current frame is done.

### DeSerialize:
In order to begin just include sweetPy header:
```c++
#include “sweetPy.h"
```
In order to deserialise two user types instances are required:

**Serialisation context** - Will provide a reference to our to be deserialise buffer.

**SweetPickle** - provides the mean to read (DeSerialise).

In order to create the required two instances a special Factory exists:
```c++
auto& factory = sweetPy::SweetPickleFactory::Instance();
auto context = factory.CreateContext(sweetPy::SerializeType::FlatBuffers);
auto sweetPickle = factory.Create(sweetPy::SerializeType::FlatBuffers);
```
The factory different methods receives a unique enum depicting our method of deserialisation, as of now only google’s flatbuffers package is supported.

SweetPickle contains a **start read** method, which will receive the created context, the received buffer and its size.
```c++
sweetPickle->StartRead(*context, buffer, size);
```
The method will take no ownership upon the received buffer.

In order to read, use the read method, an lvalue expression is required:
```c++
double val = 0;
sweetPickle->Read(val);
```

The read method will automatically advance to the next value, so a successive read invoke is all that is required in order to read the next value.

it is also possible to inquire the current value for its type:
```c++
sweetPickle->GetType();
```
