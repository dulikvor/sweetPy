#pragma once

#include <Python.h>
#include <string>
#include <vector>
#include <type_traits>
#include "core/Source.h"
#include "CPyModuleContainer.h"
#include "CPythonModule.h"
#include "CPythonRef.h"
#include "CPythonEnumValue.h"
#include "CPythonType.h"
#include "CPythonClassType.h"
#include "src/Core/Exception.h"
#include "src/Core/Traits.h"
#include "src/Core/Lock.h"
#include "src/Core/Assert.h"

namespace sweetPy{


    template<typename Type>
    class CPythonObjectType : public CPythonType {
    public:
        typedef CPythonObjectType<Type> self;

        CPythonObjectType(CPythonModule &module)
                : CPythonType(std::to_string((int)CPyModuleContainer::TypeHash<self>()) + "-GenericObjectType",
                                      std::to_string((int)CPyModuleContainer::TypeHash<self>()) + "-GenericObjectType")
        {
            ob_base.ob_base.ob_type = &CPythonMetaClass::GetStaticMetaType();
            ob_base.ob_base.ob_refcnt = 1;
            ob_base.ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(Type) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_GC;
            tp_doc = m_doc.c_str();
            tp_traverse = &Traverse;
            tp_new = PyBaseObject_Type.tp_new;
        }

    private:
        static int Traverse(PyObject *self, visitproc visit, void *arg)
        {
            //Instance members are kept out of the instance dictionary, they are part of the continuous memory of the instance, kept in C POD form.
            //There is no need to traverse the members due to the fact they are not part of python garbage collector and uknown to python.
            return 0;
        }

        static void Dealloc(PyObject* object)
        {
            //No need to call reference forget - being called by _Py_Dealloc
            PyTypeObject* type = Py_TYPE(object);

            if (PyType_IS_GC(type))
                PyObject_GC_UnTrack(object);

            if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
                Py_DECREF(type);

            ((Type*)(object + 1))->~Type();
            type->tp_free(object);
        }

    private:
    };

    //Reference types are not supported by CPythonObject
    template<typename Type, typename std::enable_if<!std::is_reference<Type>::value, bool>::type = true>
    class CPythonObject
    {
    public:
        CPythonObject(CPythonModule &module)
        : m_module(module), m_type((PyObject*)new CPythonObjectType<Type>(module), &Deleter::Owner)
        {
            Py_IncRef((PyObject *) m_type.get()); //Making sure the instance will leave outside python garbage collector
        }

        ~CPythonObject()
        {
            auto& moduleContainer = CPyModuleContainer::Instance();
            size_t key = CPyModuleContainer::TypeHash<CPythonObjectType<Type>>();
            if(moduleContainer.Exists(key) == false)
            {
                PyType_Ready((PyTypeObject*)m_type.get());
                m_module.AddType((CPythonType*)m_type.get());
                moduleContainer.AddType(key, std::move(m_type));
            }
        }

        template<typename Y>
        static PyObject* Alloc(PyTypeObject* type, Y&& object)
        {
            PyObject* newObject = type->tp_alloc(type, 0);
            new(newObject + 1)Type(std::forward<Y>(object));
            return newObject;
        }
    private:
        object_ptr m_type;
        CPythonModule &m_module;
    };


    template<typename T> struct FromNativeTypeToPyType{};
    template<> struct FromNativeTypeToPyType<int>{static constexpr void* type = &PyLong_Type;};
    template<> struct FromNativeTypeToPyType<std::string>{static constexpr void* type = &PyUnicode_Type;};
    template<> struct FromNativeTypeToPyType<const char*>{static constexpr void* type = &PyUnicode_Type;};
    template<> struct FromNativeTypeToPyType<char*>{static constexpr void* type = &PyUnicode_Type;};
    template<> struct FromNativeTypeToPyType<bool>{static constexpr void* type = &PyBool_Type;};


    template<typename T, typename = void>
    struct Object{};

    template<typename T>
    struct Object<T, typename std::enable_if<!std::is_pointer<T>::value && !is_container<T>::value && std::is_copy_constructible<T>::value &&
                                             !std::is_enum<T>::value && !std::is_reference<T>::value>::type> {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = true;
        static T& GetTyped(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            T* obj = (T*)(reinterpret_cast<PyObject*>(*reinterpret_cast<PyObject**>(fromBuffer)) + 1);
            new(toBuffer)T(*obj);
            return *reinterpret_cast<T*>(toBuffer);
        }

        static T& FromPython(PyObject* obj)
        {
            return *reinterpret_cast<T*>(obj + 1);
        }

        static PyObject* ToPython(T& obj)
        {
            auto& container = CPyModuleContainer::Instance();
            size_t classTypeKey = CPyModuleContainer::TypeHash<CPythonClassType<T>>();
            size_t objectTypeKey = CPyModuleContainer::TypeHash<CPythonObjectType<T>>();
            PyTypeObject* type = nullptr;
            if(container.Exists(classTypeKey) == true)
                type = container.GetType(classTypeKey);
            else if(container.Exists(objectTypeKey) == true)
                type = container.GetType(objectTypeKey);

            CPYTHON_VERIFY(type != nullptr, "Requested PyObjectType does not exists");
            return CPythonObject<T>::Alloc(type, obj);
        }
    };

    template<typename T>
    struct Object<T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_copy_constructible<T>::value &&
                                             !std::is_enum<T>::value && !std::is_reference<T>::value &&
                                             std::is_move_constructible<T>::value>::type> {
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = true;
        static T& GetTyped(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            T* obj = (T*)(reinterpret_cast<PyObject*>(*reinterpret_cast<PyObject**>(fromBuffer)) + 1);
            new(toBuffer)T(std::move(*obj));
            return *reinterpret_cast<T*>(toBuffer);
        }

        static T& FromPython(PyObject* obj)
        {
            return *reinterpret_cast<T*>(obj + 1);
        }

        static PyObject* ToPython(T& obj)
        {
            auto& container = CPyModuleContainer::Instance();
            size_t classTypeKey = CPyModuleContainer::TypeHash<CPythonClassType<T>>();
            size_t objectTypeKey = CPyModuleContainer::TypeHash<CPythonObjectType<T>>();
            PyTypeObject* type = nullptr;
            if(container.Exists(classTypeKey) == true)
                type = container.GetType(classTypeKey);
            else if(container.Exists(objectTypeKey) == true)
                type = container.GetType(objectTypeKey);

            CPYTHON_VERIFY(type != nullptr, "Requested PyObjectType does not exists");
            return CPythonObject<T>::Alloc(type, std::move(obj));
        }
    };

    template<typename T>
    struct Object<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    public:
        typedef int FromPythonType;
        typedef T Type;
        static constexpr const char* Format = "O";
        static const bool IsSimpleObjectType = false;
        static T& GetTyped(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            object_ptr attrName(PyUnicode_FromString("_value_"), &Deleter::Owner);
            object_ptr attr(PyObject_GetAttr(object, attrName.get()), &Deleter::Owner);
            new(toBuffer)T((T)int(PyLong_AsLongLong(attr.get())));
            return *reinterpret_cast<T*>(toBuffer);
        }
    };

    template<typename T>
    struct Object<T&>{
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static T& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* obj = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<T>(obj)){
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                return *reinterpret_cast<T*>(obj + 1);
            }
        }
        static T& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<T>(obj)){
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                return *reinterpret_cast<T*>(obj + 1);
            }
        }
        static PyObject* ToPython(T& instance){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<T>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, instance);
        }
    };

    //All python value category are basicaly lvalue, so as long as the C++ function prototype will use reference collapsing it will receive it as lvalue.
    template<typename T>
    struct Object<T&&>{
    public:
        typedef PyObject* FromPythonType;
        typedef T Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static T&& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* obj = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<T>(obj)){
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(obj + 1);
                return std::move(refObject->GetRef());
            }
            else{
                return std::move(*reinterpret_cast<T*>(obj + 1));
            }
        }
        static T&& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<T>(obj)){
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(obj + 1);
                return std::move(refObject->GetRef());
            }
            else{
                return std::move(*reinterpret_cast<T*>(obj + 1));
            }
        }
        //No meaning to return rvalue reference to python (only supports lvalue value category), so ToPython is not implemented.
    };

    template<typename T>
    struct Object<std::vector<T>>{
    public:
        typedef PyObject* FromPythonType;
        typedef std::vector<T> Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static std::vector<T>& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* pyListObject = *reinterpret_cast<PyObject**>(fromBuffer);
            CPYTHON_VERIFY(pyListObject->ob_type == &PyList_Type, "It is mandatory for the received type to be of PyList_Type");
            Py_ssize_t numOfElements = PyList_Size(pyListObject);
            new(toBuffer)std::vector<T>;
            std::vector<T>& vectorObject = *(std::vector<T>*)(toBuffer);
            vectorObject.reserve(numOfElements);
            for(int index = 0; index < numOfElements; index++)
            {
                PyObject* element = PyList_GetItem(pyListObject, index);
                CPYTHON_VERIFY(element->ob_type == FromNativeTypeToPyType<T>::type, "PyListObject type must match a transition to type T");
                vectorObject.emplace_back(Object<T>::FromPython(element));
            }
            return *reinterpret_cast<std::vector<T>*>(toBuffer);
        }

        static std::vector<T> FromPython(PyObject* object){
            GilLock lock;
            CPYTHON_VERIFY(object->ob_type == &PyList_Type, "It is mandatory for the received type to be of PyList_Type");
            Py_ssize_t numOfElements = PyList_Size(object);
            std::vector<T> vectorObject;
            vectorObject.reserve(numOfElements);
            for(int index = 0; index < numOfElements; index++)
            {
                PyObject* element = PyList_GetItem(object, index);
                CPYTHON_VERIFY(element->ob_type == FromNativeTypeToPyType<T>::type, "PyListObject type must match a transition to type T");
                vectorObject.emplace_back(Object<T>::FromPython(element));
            }
            return vectorObject;
        }

        static PyObject* ToPython(const std::vector<T>& object){
            PyObject* pyListObject = PyList_New(object.size());
            for( int index = 0; index < object.size(); index++)
                PyList_SetItem(pyListObject, index, Object<T>::ToPython(object[index]));

            return pyListObject;
        }
    };

    template<typename T>
    struct Object<std::vector<T>&>{
    public:
        typedef PyObject* FromPythonType;
        typedef std::vector<T> Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static std::vector<T>& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<std::vector<T>>(object))
            {
                CPythonRefObject<std::vector<T>>* refObject = reinterpret_cast<CPythonRefObject<std::vector<T>>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "std::vector can only originates from ref to std::vector type");
        }

        static std::vector<T>& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<std::vector<T>>(obj)){
                CPythonRefObject<std::vector<T>>* refObject = reinterpret_cast<CPythonRefObject<std::vector<T>>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python list object to std::vector is not possible");
            }
        }

        static PyObject* ToPython(std::vector<T>& data){ //Only l_value, no xpire value
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<std::vector<T>>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<typename T>
    struct Object<const std::vector<T>&>{
    public:
        typedef PyObject* FromPythonType;
        typedef std::vector<T> Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static const std::vector<T>& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(object->ob_type == &PyList_Type)
            {
                Py_ssize_t numOfElements = PyList_Size(object);
                new(toBuffer)std::vector<T>;
                std::vector<T>& vectorObject = *(std::vector<T>*)(toBuffer);
                vectorObject.reserve(numOfElements);
                for(int index = 0; index < numOfElements; index++)
                {
                    PyObject* element = PyList_GetItem(object, index);
                    CPYTHON_VERIFY(element->ob_type == FromNativeTypeToPyType<T>::type, "PyListObject type must match a transition to type T");
                    vectorObject.emplace_back(Object<T>::FromPython(element));
                }
                return *reinterpret_cast<std::vector<T>*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<const std::vector<T>>(object))
            {
                CPythonRefObject<const std::vector<T>>* refObject = reinterpret_cast<CPythonRefObject<const std::vector<T>>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "std::vector can only originates from python list type or ref to std::vector type");
        }

        static const std::vector<T>& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<std::vector<T>>(obj)){
                CPythonRefObject<const std::vector<T>>* refObject = reinterpret_cast<CPythonRefObject<const std::vector<T>>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python list object to std::vector is not possible");
            }
        }

        static PyObject* ToPython(const std::vector<T>& data){ //Only l_value, no xpire value
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<const std::vector<T>>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<const char*> {
    public:
        typedef const char* FromPythonType;
        typedef const char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "z";
        static const char*& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)const char*(*reinterpret_cast<char**>(fromBuffer));
            return *reinterpret_cast<const char**>(toBuffer);
        }
        static const char* FromPython(PyObject* object){
            GilLock lock;
            const char* str;
            if(Py_TYPE(object) == &PyUnicode_Type)
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python unicode string to const char* is not possible");
            else if(Py_TYPE(object) == &PyBytes_Type)
                str = PyBytes_AsString(object);

            return str;
        }
        static PyObject* ToPython(const char* data){
            return PyBytes_FromString(data);
        }
    };

    template<>
    struct Object<char*> {
    public:
        typedef const char* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "z";
        static char*& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)char*(*reinterpret_cast<char**>(fromBuffer));
            return *reinterpret_cast<char**>(toBuffer);
        }
        static char* FromPython(PyObject* object){
            GilLock lock;
            char* str;
            if(Py_TYPE(object) == &PyUnicode_Type)
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python unicode string to char* is not possible");
            else if(Py_TYPE(object) == &PyBytes_Type)
                str = PyBytes_AsString(object);

            return str;
        }
        static PyObject* ToPython(char* data){
            return PyBytes_FromString(data);
        }
    };

    template<size_t N>
    struct Object<char[N]> {
    public:
        typedef const char* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "z";
        static char (&GetTyped(char* fromBuffer, char* toBuffer))[N]{
            CPYTHON_VERIFY(strlen(fromBuffer) <= N-1, "Python string size is too long for char array.");
            new(toBuffer)char*(*reinterpret_cast<char**>(fromBuffer));
            return static_cast<char[N]>(toBuffer);
        }
        static char(&FromPython(PyObject* object))[N]{
            GilLock lock;
            char* str;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr && strlen(PyBytes_AsString(bytesObject.get())) <= N-1);
                str = new char[N];
                memcpy(str, PyBytes_AsString(bytesObject.get()), N - 1);
                str[N - 1] = '\0';
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                CPYTHON_VERIFY_EXC(strlen(PyBytes_AsString(object)) <= N - 1);
                str = new char[N];
                memcpy(str, PyBytes_AsString(object), N - 1);
                str[N - 1] = '\0';
                str = PyBytes_AsString(object);
            }
            return static_cast<char[N]>(str);
        }
        static PyObject* ToPython(char(&data)[N]){
            return PyBytes_FromString(data);
        }
    };

    template<size_t N>
    struct Object<const char[N]> {
    public:
        typedef const char* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "z";
        static const char (&GetTyped(char* fromBuffer, char* toBuffer))[N]{
            CPYTHON_VERIFY(strlen(fromBuffer) <= N-1, "Python string size is too long for char array.");
            new(toBuffer)char*(*reinterpret_cast<char**>(fromBuffer));
            return static_cast<const char[N]>(toBuffer);
        }
        static const char(&FromPython(PyObject* object))[N]{
            GilLock lock;
            char* str;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                str = PyBytes_AsString(bytesObject.get());
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
                str = PyBytes_AsString(object);

            CPYTHON_VERIFY(strlen(str) <= N-1, "Python string size is too long for char array.");
            return static_cast<const char[N]>(str);
        }
        static PyObject* ToPython(const char(&data)[N]){
            return PyBytes_FromString(data);
        }
    };

    template<>
    struct Object<std::string> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                new(toBuffer)std::string(PyBytes_AsString(object));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object))
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static std::string FromPython(PyObject* object){
            GilLock lock;
            std::string result;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                result = PyBytes_AsString(bytesObject.get());
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
                result = PyBytes_AsString(object);

            return result;
        }
        static PyObject* ToPython(const std::string& data){
            return PyBytes_FromString(data.c_str());
        }
    };
    //Providing lvalue string is not possible, due to transform between two types, so FromPython is removed.
    template<>
    struct Object<const std::string&> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const std::string& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                new(toBuffer)std::string(PyBytes_AsString(object));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<const std::string>(object))
            {
                CPythonRefObject<const std::string>* refObject = reinterpret_cast<CPythonRefObject<const std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const string& can only originates from python string or ref string type");
        }
        static const std::string& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<const std::string>(obj)){
                CPythonRefObject<const std::string>* refObject = reinterpret_cast<CPythonRefObject<const std::string>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python string to const string& is not possible");
            }
        }
        static PyObject* ToPython(const std::string& data){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<const std::string>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<std::string&> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(CPythonRef<>::IsReferenceType<std::string>(object)) //Only ref string type is supported due to possible scope leakage
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from ref string type");
        }
        static std::string& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<std::string>(obj)){
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(obj + 1);
                return refObject->GetRef();
            }
            else{
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python string to string& is not possible");
            }
        }
        static PyObject* ToPython(std::string& data){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<std::string>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<std::string&&> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string&& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                new(toBuffer)std::string(PyBytes_AsString(bytesObject.get()));
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                new(toBuffer)std::string(PyBytes_AsString(object));
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object))
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python CPythonRef to string&& is not possible");
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& can only originates from python string");
        }
        static std::string&& FromPython(PyObject* object){
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& is not supported");
        }
        static PyObject* ToPython(std::string&& data){
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Transform of to be expired value to python is not possible");
        }
    };

    template<>
    struct Object<bool> {
    public:
        typedef long FromPythonType;
        typedef bool Type;
        static constexpr const char *Format = "l";
        static const bool IsSimpleObjectType = false;
        static bool& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)bool(*reinterpret_cast<bool*>(fromBuffer));
            return *reinterpret_cast<bool*>(toBuffer);
        }
        static bool FromPython(PyObject* obj){
            GilLock lock;
            return (bool)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(const bool& data){
            return PyBool_FromLong(data);
        }
    };

    template<>
    struct Object<int> {
    public:
        typedef long FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)int(PyLong_AsLongLong(object));
                return *reinterpret_cast<int*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<int>(object))
            {
                CPythonRefObject<int>* refObject = reinterpret_cast<CPythonRefObject<int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type or python long object");
        }
        static int FromPython(PyObject* obj){
            GilLock lock;
            return (int)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(const int& data){
            return PyLong_FromLong(data);
        }
    };

    template<>
    struct Object<const int&> {
    public:
        typedef long FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const int& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyLong_Type)
            {
                new(toBuffer)int(PyLong_AsLong(object));
                return *reinterpret_cast<int*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<const int>(object))
            {
                CPythonRefObject<const int>* refObject = reinterpret_cast<CPythonRefObject<const int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type or python long object");
        }
        //Conversion from python long to const int& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static const int& FromPython(PyObject* object){
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<const int>(object)) {
                CPythonRefObject<const int> *refObject = reinterpret_cast<CPythonRefObject<const int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type");
        }
        static PyObject* ToPython(const int& data){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<const int>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<int&> {
    public:
        typedef long FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(CPythonRef<>::IsReferenceType<int>(object))
            {
                CPythonRefObject<int>* refObject = reinterpret_cast<CPythonRefObject<int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int& can only originates from ref int type");
        }
        static int& FromPython(PyObject* object){
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<int>(object)) {
                CPythonRefObject<int> *refObject = reinterpret_cast<CPythonRefObject<int> *>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int& can only originates from ref int type");
        }
        static PyObject* ToPython(const int& data){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<int>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<int&&> {
    public:
        typedef long FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int&& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyLong_Type) {
                new(toBuffer)int(PyLong_AsLong(object));
                return std::move(*reinterpret_cast<int*>(toBuffer));
            }
            else if(CPythonRef<>::IsReferenceType<int>(object))
            {
                CPythonRefObject<int>* refObject = reinterpret_cast<CPythonRefObject<int>*>(object + 1);
                return std::move(refObject->GetRef());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int&& can only originates from ref int type or py long type");
        }
        //Conversion from python long to int&& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static int&& FromPython(PyObject* object){
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<int>(object)) {
                CPythonRefObject<int> *refObject = reinterpret_cast<CPythonRefObject<int> *>(object + 1);
                return std::move(refObject->GetRef());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "int&& can only originates from ref int type");
        }
        static PyObject* ToPython(const int& data){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<int>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<PyObject*> {
    public:
        static PyObject* ToPython(PyObject*& data){
            Py_XINCREF(data);
            return data;
        }
    };

    template<typename... Args>
    struct ObjectsPackSize{};

    template<>
    struct ObjectsPackSize<>
    {
        static const int value = 0;
    };

    template<typename Type, typename... Args>
    struct ObjectsPackSize<Type, Args...>
    {
        static const int value = sizeof(Type) + ObjectsPackSize<Args...>::value;
    };

    enum OffsetType : short
    {
        FromPython,
        ToNative
    };

    template<OffsetType, typename... Args>
    struct ObjectOffset{};


    template<typename T, typename... Args>
    struct ObjectOffset<FromPython, T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename... Args>
    struct ObjectOffset<ToNative, T, T, Args...>
    {
        static const int value = 0;
    };

    template<typename T, typename X, typename... Args>
    struct ObjectOffset<FromPython, T, X, Args...>
    {
        static const int value = ObjectOffset<FromPython, T, Args...>::value + sizeof(typename X::FromPythonType);
    };

    template<typename T, typename X, typename... Args>
    struct ObjectOffset<ToNative, T, X, Args...>
    {
        static const int value = ObjectOffset<ToNative, T, Args...>::value + sizeof(typename X::Type);
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper
    {

        typedef typename Object<T>::FromPythonType FromPythonType;
        typedef typename Object<T>::Type Type;
        static void* AllocateObjectType(CPythonModule& module) {
            if(Object<T>::IsSimpleObjectType == true)
               CPythonObject<T> type(module);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            Type* typedPtr = reinterpret_cast<Type*>(buffer);
            typedPtr->~Type();
            return nullptr;
        }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<T&, I>
    {
        typedef typename Object<T&>::FromPythonType FromPythonType;
        typedef typename Object<T&>::Type Type;
        static void* AllocateObjectType(CPythonModule& module) {}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            return nullptr;
        }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<T&&, I>
    {
        typedef typename Object<T&&>::FromPythonType FromPythonType;
        typedef typename Object<T&&>::Type Type;
        static void* AllocateObjectType(CPythonModule& module) {}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            return nullptr;
        }
    };

    template<std::size_t I, size_t N>
    struct ObjectWrapper<char[N], I>
    {
        typedef typename Object<char[N]>::FromPythonType FromPythonType;
        typedef typename Object<char[N]>::Type Type;
        static void* AllocateObjectType(CPythonModule& module) {}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            return nullptr;
        }
    };

    template<std::size_t I>
    struct ObjectWrapper<void, I>
    {
        static void* AllocateObjectType(CPythonModule& module) {}
    };
}
