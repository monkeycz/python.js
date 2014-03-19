
#ifndef PY_OBJECT_WRAPPER_H
#define PY_OBJECT_WRAPPER_H

#include <node.h>
#include <Python.h>

#include "utils.h"

using namespace v8;

class PyObjectWrapper : public node::ObjectWrap
{
public:
    PyObjectWrapper(PyObject* py_object);
    virtual ~PyObjectWrapper();

    static void Initialize();

    static PyObject* ConvertToPython(const Handle<Value>& js_value);

    static Handle<Value> New(PyObject* py_object);
    static Handle<Value> New(const Arguments& js_args);

    static Handle<Value> NamedGetter(Local<String> js_key, const AccessorInfo& js_info);
    static Handle<Value> NamedSetter(Local<String> js_key, Local<Value> js_value, const AccessorInfo& js_info);
    static Handle<Value> IndexedGetter(uint32_t index, const AccessorInfo& js_info);
    static Handle<Value> IndexedSetter(uint32_t index, Local<Value> js_value, const AccessorInfo& js_info);
    static Handle<Array> Enumerator(const AccessorInfo& js_info);

    static Handle<Value> CallAccessor(Local<String> js_property, const AccessorInfo& js_info);
    static Handle<Value> ValueOfAccessor(Local<String> js_property, const AccessorInfo& js_info);
    static Handle<Value> ToStringAccessor(Local<String> js_property, const AccessorInfo& js_info);

    static Handle<Value> Call(const Arguments& js_args);
    static Handle<Value> ValueOf(const Arguments& js_args);
    static Handle<Value> ToString(const Arguments& js_args);

    PyObject* InstanceGetPyObject();

    Handle<Value> InstanceNamedGetter(Local<String> js_key);
    Handle<Value> InstanceNamedSetter(Local<String> js_key, Local<Value> js_value);
    Handle<Value> InstanceIndexedGetter(uint32_t index);
    Handle<Value> InstanceIndexedSetter(uint32_t index, Local<Value> js_value);
    Handle<Array> InstanceEnumerator();

    Handle<Value> InstanceCall(const Arguments& js_args);
    Handle<Value> InstanceValueOf(const Arguments& js_args);
    Handle<Value> InstanceValueOf(PyObject* py_object);
    Handle<Value> InstanceToString(const Arguments& js_args);

public:
    static Persistent<FunctionTemplate> py_function_template;

private:
    static PyTypeObject PyCFunction_Type_x;

    static void py_method_init_x(void);
    static void py_method_dealloc_x(PyObject* py_object);

    static PyObject* py_method_function_x(PyObject* py_self, PyObject* py_args);

private:
    PyObject* __py_object;
};

#endif /* PY_OBJECT_WRAPPER_H */
