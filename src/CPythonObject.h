#pragma once

#include <string>
#include <type_traits>
#include <Python.h>
#include "core/Source.h"
#include "Lock.h"
#include "CPyModuleContainer.h"
#include "CPythonModule.h"
#include "CPythonRef.h"
#include "CPythonEnumValue.h"
#include "CPythonType.h"
#include "CPythonClassType.h"
#include "Exception.h"

namespace sweetPy{


    template<typename Type>
    class CPythonObjectType : public CPythonType {
    public:
        typedef CPythonObjectType<Type> self;

        CPythonObjectType(CPythonModule &module)
                : CPythonType(std::to_string((int)CPyModuleContainer::TypeHash<self>()) + "-GenericObjectType",
                                      std::to_string((int)CPyModuleContainer::TypeHash<self>()) + "-GenericObjectType")
        {
            ob_type = &CPythonMetaClass<>::GetStaticMetaType();
            ob_refcnt = 1;
            ob_size = 0;
            tp_name = m_name.c_str();
            tp_basicsize = sizeof(Type) + sizeof(PyObject);
            tp_dealloc = &Dealloc;
            tp_flags = Py_TPFLAGS_HAVE_CLASS |
                       Py_TPFLAGS_HAVE_GC;
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
        : m_module(module), m_type(new CPythonObjectType<Type>(module))
        {
            Py_IncRef((PyObject *) m_type.get()); //Making sure the instance will leave outside python garbage collector
        }

        ~CPythonObject()
        {
            PyType_Ready(m_type.get());
            CPyModuleContainer::Instance().AddType(CPyModuleContainer::TypeHash<CPythonObjectType<Type>>(), m_type.get());
            m_module.AddType(std::move(m_type));
        }

        template<typename Y>
        static PyObject* Alloc(PyTypeObject* type, Y&& object)
        {
            PyObject* newObject = type->tp_alloc(type, 0);
            new(newObject + 1)Type(std::forward<Y>(object));
            return newObject;
        }
    private:
        std::unique_ptr<CPythonType> m_type;
        CPythonModule &m_module;
    };

    template<typename T, typename = void>
    struct Object{};

    template<typename T>
    struct Object<T, typename std::enable_if<!std::is_pointer<T>::value && std::is_copy_constructible<T>::value &&
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
        static constexpr const char* Format = "i";
        static const bool IsSimpleObjectType = false;
        static T& GetTyped(char* fromBuffer, char* toBuffer) //Non python types representation - PyPbject Header + Native data
        {
            new(toBuffer)T((T)*reinterpret_cast<int*>(fromBuffer));
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
        static const char* FromPython(PyObject* obj){
            GilLock lock;
            return PyString_AsString(obj);
        }
        static PyObject* ToPython(const char* data){
            return PyString_FromString(data);
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
            if(Py_TYPE(object) == &PyBaseString_Type)
            {
                new(toBuffer)std::string(PyString_AsString(object));
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
        static std::string FromPython(PyObject* obj){
            GilLock lock;
            char* data = PyString_AsString(obj); // Providing its underline buffer, a copy is in need.
            return std::string(data);
        }
        static PyObject* ToPython(const std::string& data){
            return PyString_FromString(data.c_str());
        }
    };
    //Providing lvalue string is not possible, due to transform between two types, so FromPython is removed.
    template<>
    struct Object<std::string&> {
    public:
        typedef const char* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyString_Type)
            {
                new(toBuffer)std::string(PyString_AsString(object));
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object))
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from python string or ref string type");
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
            if(Py_TYPE(object) == &PyString_Type)
            {
                new(toBuffer)std::string(PyString_AsString(object));
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object))
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion between python CPythonRef to string&& is not possible");
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& can only originates from python string");
        }
        static std::string&& FromPython(PyObject* object){
            if(Py_TYPE(object) == &PyString_Type)
                return std::string(PyString_AsString(object));
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& can only originates from python string");
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
        static constexpr const char *Format = "l";
        static const bool IsSimpleObjectType = false;
        static int& GetTyped(char* fromBuffer, char* toBuffer){
            new(toBuffer)int(*reinterpret_cast<int*>(fromBuffer));
            return *reinterpret_cast<int*>(toBuffer);
        }
        static int FromPython(PyObject* obj){
            GilLock lock;
            return (int)PyLong_AsLong(obj);
        }
        static PyObject* ToPython(const int& data){
            return PyInt_FromLong(data);
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

    template<std::size_t I>
    struct ObjectWrapper<void, I>
    {
        static void* AllocateObjectType(CPythonModule& module) {}
    };
}
