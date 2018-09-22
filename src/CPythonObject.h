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
        : m_module(module), m_type(new CPythonObjectType<Type>(module))
        {
            Py_IncRef((PyObject *) m_type.get()); //Making sure the instance will live outside python's garbage collector
        }

        ~CPythonObject()
        {
            auto& moduleContainer = CPyModuleContainer::Instance();
            size_t key = CPyModuleContainer::TypeHash<CPythonObjectType<Type>>();
            if(moduleContainer.Exists(key) == false)
            {
                PyType_Ready((PyTypeObject*)m_type.get());
                m_module.AddType((CPythonType*)m_type.get());

                moduleContainer.AddType(key, object_ptr((PyObject*)m_type.release(), &Deleter::Owner));

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
        std::unique_ptr<CPythonObjectType<Type>> m_type;
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
        typedef void* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static T& GetTyped(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<T>(object))
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1); //Peircing const modifier
                return refObject->GetRef();
            }
            else
                return *reinterpret_cast<T*>(object + 1);
        }
        static T& FromPython(PyObject* object)
        {
            if(CPythonRef<>::IsReferenceType<T>(object))
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1); //Peircing const modifier
                return refObject->GetRef();
            }
            else
                return *reinterpret_cast<T*>(object + 1);
        }
        static PyObject* ToPython(T& instance)
        {
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<T>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, instance);
        }
    };

    template<typename T>
    struct Object<const T&>{
    public:
        typedef PyObject* FromPythonType;
        typedef void* Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static const T& GetTyped(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<T>(object))
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<const T>(object))
            {
                CPythonRefObject<const T>* refObject = reinterpret_cast<CPythonRefObject<const T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<const T>* refObject = reinterpret_cast<CPythonRefObject<const T>*>(object + 1); //Peircing const modifier
                return refObject->GetRef();
            }
            else
                return *reinterpret_cast<T*>(object + 1);
        }
        static const T& FromPython(PyObject* object)
        {
            if(CPythonRef<>::IsReferenceType<T>(object))
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<const T>(object))
            {
                CPythonRefObject<const T>* refObject = reinterpret_cast<CPythonRefObject<const T>*>(object + 1);
                return refObject->GetRef();
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<const T>* refObject = reinterpret_cast<CPythonRefObject<const T>*>(object + 1);
                return refObject->GetRef();
            }
            else
                return *reinterpret_cast<T*>(object + 1);
        }
        static PyObject* ToPython(const T& instance)
        {
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<const T>>();
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
            PyObject* object = *reinterpret_cast<PyObject**>(fromBuffer);
            if(CPythonRef<>::IsReferenceType<T>(object)){
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return std::move(refObject->GetRef());
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1); //Peircing const modifier
                return refObject->GetRef();
            }
            else
            {
                return std::move(*reinterpret_cast<T*>(object + 1));
            }
        }
        static T&& FromPython(PyObject* object)
        {
            if(CPythonRef<>::IsReferenceType<T>(object))
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1);
                return std::move(refObject->GetRef());
            }
            else if(&CPythonRef<>::GetStaticType() == object->ob_type)
            {
                CPythonRefObject<T>* refObject = reinterpret_cast<CPythonRefObject<T>*>(object + 1); //Peircing const modifier
                return refObject->GetRef();
            }
            else
                return std::move(*reinterpret_cast<T*>(object + 1));
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
        typedef PyObject* FromPythonType;
        typedef const char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "z";
        static const char* GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                size_t length = PyBytes_Size(bytesObject.get());
                fromBuffer = new char[length];
                memcpy(fromBuffer, PyBytes_AsString(bytesObject.get()), length);
                return fromBuffer;
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                fromBuffer = new char[length];
                memcpy(fromBuffer, PyBytes_AsString(object), length);
                return fromBuffer;
            }
            else if(CPythonRef<>::IsReferenceType<const char*>(object))
            {
                CPythonRefObject<const char*>* refObject = reinterpret_cast<CPythonRefObject<const char*>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static const char* FromPython(PyObject* object){
            GilLock lock;
            if(Py_TYPE(object) == &PyBytes_Type)
                return PyBytes_AsString(object);
            else if(CPythonRef<>::IsReferenceType<const char*>(object))
            {
                CPythonRefObject<const char*>* refObject = reinterpret_cast<CPythonRefObject<const char*>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion to const char* is only allowed from python's byte array, ref const char* wrapper or ref char* wrapper");
        }
        static PyObject* ToPython(const char* data){
            return PyBytes_FromString(data);
        }
    };

    template<>
    struct Object<char*> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static char* GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                size_t length = PyBytes_Size(bytesObject.get());
                fromBuffer = new char[length];
                memcpy(fromBuffer,PyBytes_AsString(bytesObject.get()), length);
                return fromBuffer;
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                fromBuffer = new char[length];
                memcpy(fromBuffer,PyBytes_AsString(object), length);
                return fromBuffer;
            }
            else if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string can only originates from python string or ref string type");
        }
        static char* FromPython(PyObject* object){
            GilLock lock;
            if(Py_TYPE(object) == &PyBytes_Type)
                return PyBytes_AsString(object);
            else if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion to char* is only allowed from python's byte array or ref char* wrapper");
        }
        static PyObject* ToPython(char* data){
            return PyBytes_FromString(data);
        }
    };

    template<size_t N>
    struct Object<char[N]> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
    };

    template<size_t N>
    struct Object<const char[N]> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
    };

    template<size_t N>
    struct Object<char(&)[N]> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static char (&GetTyped(char* fromBuffer, char* toBuffer))[N]{
            PyObject* object = *(PyObject**)fromBuffer;
            //Conversion from python unicode and bytecode will be supported due to limitation of scope of returned value.
            if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                CPYTHON_VERIFY(strlen(refObject->GetRef()) <= N-1, "Python string size is too long for char array.");
                return (char(&)[N])(refObject->GetRef());
            }
            else if(CPythonRef<>::IsReferenceType<char[N]>(object))
            {
                CPythonRefObject<char[N]>* refObject = reinterpret_cast<CPythonRefObject<char[N]>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "char array can only originates from python's unicode string, bytes array or integral types - char* or char[N]");
        }
        static char(&FromPython(PyObject* object))[N]
        {
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<char[N]>(object))
            {
                CPythonRefObject<char[N]>* refObject = reinterpret_cast<CPythonRefObject<char[N]>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Only char[N]& is supported.");
        }

        static PyObject* ToPython(char(&data)[N]){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<char[N]>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<size_t N>
    struct Object<const char(&)[N]> {
    public:
        typedef PyObject* FromPythonType;
        typedef char* Type;
        static const bool IsSimpleObjectType = false;
        static constexpr const char *Format = "O";
        static const char (&GetTyped(char* fromBuffer, char*&& toBuffer))[N]{
            PyObject* object = *(PyObject**)fromBuffer;
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                char* bytesBuffer = PyBytes_AsString(bytesObject.get());

                size_t length = strlen(bytesBuffer);
                CPYTHON_VERIFY(length <= N-1, "Python string size is too long for char array.");
                toBuffer = new char[N];
                memcpy(toBuffer, bytesBuffer, length);
                toBuffer[length] = '\0';

                return (const char(&)[N])toBuffer;
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                CPYTHON_VERIFY(length <= N, "Python string size is too long for char array.");
                char* bytesBuffer = PyBytes_AsString(object);

                toBuffer = new char[N];
                memcpy(toBuffer, bytesBuffer, length);
                return (const char(&)[N])toBuffer;
            }
            else if(CPythonRef<>::IsReferenceType<char*>(object))
            {
                CPythonRefObject<char*>* refObject = reinterpret_cast<CPythonRefObject<char*>*>(object + 1);
                CPYTHON_VERIFY(strlen(refObject->GetRef()) <= N-1, "Python string size is too long for char array.");
                return (const char(&)[N])(refObject->GetRef());
            }
            else if(CPythonRef<>::IsReferenceType<const char*>(object))
            {
                CPythonRefObject<const char*>* refObject = reinterpret_cast<CPythonRefObject<const char*>*>(object + 1);
                CPYTHON_VERIFY(strlen(refObject->GetRef()) <= N-1, "Python string size is too long for char array.");
                return (const char(&)[N])(refObject->GetRef());
            }
            else if(CPythonRef<>::IsReferenceType<char[N]>(object))
            {
                CPythonRefObject<char[N]>* refObject = reinterpret_cast<CPythonRefObject<char[N]>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<const char[N]>(object))
            {
                CPythonRefObject<const char[N]>* refObject = reinterpret_cast<CPythonRefObject<const char[N]>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "char array can only originates from python's unicode string, bytes array or integral types - char*, const char*, const char[N] or char[N]");
        }
        static const char(&FromPython(PyObject* object))[N]
        {
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<const char[N]>(object))
            {
                CPythonRefObject<const char[N]>* refObject = reinterpret_cast<CPythonRefObject<const char[N]>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "Only const char[N]& is supported.");
        }
        static PyObject* ToPython(const char(&data)[N]){
            size_t key = CPyModuleContainer::TypeHash<CPythonRefType<const char[N]>>();
            auto& container = CPyModuleContainer::Instance();
            PyTypeObject* type = container.Exists(key) ? container.GetType(key) : &CPythonRef<>::GetStaticType();
            return CPythonRef<>::Alloc(type, data);
        }
    };

    template<>
    struct Object<std::string> {
    public:
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string GetTyped(char* fromBuffer, char* toBuffer){
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
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
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
            if(Py_TYPE(object) == &PyUnicode_Type)
            {
                object_ptr bytesObject(PyUnicode_AsASCIIString(object), &Deleter::Owner);
                CPYTHON_VERIFY_EXC(bytesObject.get() != nullptr);
                return std::string(PyBytes_AsString(bytesObject.get()));
            }
            else if(Py_TYPE(object) == &PyBytes_Type)
            {
                size_t length = PyBytes_Size(object);
                return std::string(PyBytes_AsString(object), length);
            }
            else if(CPythonRef<>::IsReferenceType<const std::string>(object)){
                CPythonRefObject<const std::string>* refObject = reinterpret_cast<CPythonRefObject<const std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object)){
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "conversion is only legit from python's unicode string, bytes array or std::string, const std::string ref wrapper object");
        }
        static PyObject* ToPython(const std::string& data){
            return PyBytes_FromString(data.c_str());
        }
    };
    //Providing lvalue string is not possible, due to transform between two types, so FromPython is removed.
    template<>
    struct Object<const std::string&> {
    public:
        typedef PyObject* FromPythonType;
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
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
                return *reinterpret_cast<std::string*>(toBuffer);
            }
            else if(CPythonRef<>::IsReferenceType<const std::string>(object))
            {
                CPythonRefObject<const std::string>* refObject = reinterpret_cast<CPythonRefObject<const std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object))
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const string& can only originates from python's unicode string, bytes array or ref string, ref const string type");
        }
        static const std::string& FromPython(PyObject* obj){
            if(CPythonRef<>::IsReferenceType<const std::string>(obj)){
                CPythonRefObject<const std::string>* refObject = reinterpret_cast<CPythonRefObject<const std::string>*>(obj + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(obj)){
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(obj + 1);
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
        typedef PyObject* FromPythonType;
        typedef std::string Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static std::string& GetTyped(char* fromBuffer, char* toBuffer)
        {
            PyObject* object = *(PyObject**)fromBuffer;
            if(CPythonRef<>::IsReferenceType<std::string>(object)) //Only ref string type is supported due to possible scope leakage
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from ref string type");
        }
        static std::string& FromPython(PyObject* obj)
        {
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<std::string>(obj))
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(obj + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string& can only originates from ref string wrapper");
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
        typedef PyObject* FromPythonType;
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
                size_t length = PyBytes_Size(object);
                new(toBuffer)std::string(PyBytes_AsString(object), length);
                return std::move(*reinterpret_cast<std::string*>(toBuffer));
            }
            else if(CPythonRef<>::IsReferenceType<std::string>(object)) //Only ref string type is supported due to possible scope leakage
            {
                CPythonRefObject<std::string>* refObject = reinterpret_cast<CPythonRefObject<std::string>*>(object + 1);
                return std::move(refObject->GetRef());
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "string&& can only originates from python string or ref string wrapper");
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
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int GetTyped(char* fromBuffer, char* toBuffer){
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
            else if(CPythonRef<>::IsReferenceType<const int>(object))
            {
                CPythonRefObject<const int>* refObject = reinterpret_cast<CPythonRefObject<const int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type, ref const int type or python long object");
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
        typedef PyObject* FromPythonType;
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
            else if(CPythonRef<>::IsReferenceType<int>(object))
            {
                CPythonRefObject<int>* refObject = reinterpret_cast<CPythonRefObject<int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int wrapper type, ref const int wrapper type or python long object");
        }
        //Conversion from python long to const int& will not be supported, due to the fact that returning rvalue encpasulate leakage scope potential.
        static const int& FromPython(PyObject* object){
            GilLock lock;
            if(CPythonRef<>::IsReferenceType<const int>(object)) {
                CPythonRefObject<const int> *refObject = reinterpret_cast<CPythonRefObject<const int>*>(object + 1);
                return refObject->GetRef();
            }
            else if(CPythonRef<>::IsReferenceType<int>(object)) {
                CPythonRefObject<int> *refObject = reinterpret_cast<CPythonRefObject<int>*>(object + 1);
                return refObject->GetRef();
            }
            else
                throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "const int& can only originates from ref int type or ref const int type");
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
        typedef PyObject* FromPythonType;
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
        typedef PyObject* FromPythonType;
        typedef int Type;
        static constexpr const char *Format = "O";
        static const bool IsSimpleObjectType = false;
        static int&& GetTyped(char* fromBuffer, char* toBuffer){
            PyObject* object = *(PyObject**)fromBuffer;
            if(CPythonRef<>::IsReferenceType<int>(object))
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
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no python representation for int&&");
        }
        static PyObject* ToPython(int&& data){
            throw CPythonException(PyExc_TypeError, __CORE_SOURCE, "no conversion from int&& to python");
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
        static void* AllocateType(CPythonModule& module) {
            if(Object<T>::IsSimpleObjectType == true)
               CPythonObject<T> type(module);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer)
        {
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
        static void* AllocateType(CPythonModule& module){}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<const T&, I>
    {
        typedef typename Object<const T&>::FromPythonType FromPythonType;
        typedef typename Object<const T&>::Type Type;
        static void* AllocateType(CPythonModule& module){}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<typename T, std::size_t I>
    struct ObjectWrapper<T&&, I>
    {
        typedef typename Object<T&&>::FromPythonType FromPythonType;
        typedef typename Object<T&&>::Type Type;
        static void* AllocateType(CPythonModule& module) {}
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            return nullptr;
        }
    };

    template<std::size_t I, size_t N>
    struct ObjectWrapper<char(&)[N], I>
    {
        typedef typename Object<char[N]>::FromPythonType FromPythonType;
        typedef typename Object<char[N]>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "char_array_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<char[N]>>()))
                CPythonRef<char[N]>(module, name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            return nullptr;
        }
    };

    template<std::size_t I, size_t N>
    struct ObjectWrapper<const char(&)[N], I>
    {
        typedef typename Object<const char[N]>::FromPythonType FromPythonType;
        typedef typename Object<const char[N]>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "const_char_array_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<const char[N]>>()))
                CPythonRef<const char[N]>(module,name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){
            delete [] buffer;
            return nullptr;
        }
    };

    template<std::size_t I>
    struct ObjectWrapper<std::string&, I>
    {
        typedef typename Object<std::string&>::FromPythonType FromPythonType;
        typedef typename Object<std::string&>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "string_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<std::string>>()))
                CPythonRef<std::string>(module, name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const std::string&, I>
    {
        typedef typename Object<const std::string&>::FromPythonType FromPythonType;
        typedef typename Object<const std::string&>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "const_string_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<const std::string>>()))
                CPythonRef<const std::string>(module, name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<int&, I>
    {
        typedef typename Object<int&>::FromPythonType FromPythonType;
        typedef typename Object<int&>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "int_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<int>>()))
                CPythonRef<int>(module, name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<const int&, I>
    {
        typedef typename Object<const int&>::FromPythonType FromPythonType;
        typedef typename Object<const int&>::Type Type;
        static void* AllocateType(CPythonModule& module)
        {
            static std::string name = "const_int_ref";
            if(!CPyModuleContainer::Instance().Exists(CPyModuleContainer::TypeHash<CPythonRefType<const int>>()))
                CPythonRef<const int>(module, name, name);
        }
        template<typename... Args>
        static void MultiInvoker(Args&&...){}
        static void* Destructor(char* buffer){ return nullptr; }
    };

    template<std::size_t I>
    struct ObjectWrapper<void, I>
    {
        static void* AllocateType(CPythonModule& module) {}
    };
}
