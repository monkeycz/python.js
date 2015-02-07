
#ifndef PY_OBJECT_WRAPPER_H
#define PY_OBJECT_WRAPPER_H

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <Python.h>

using namespace v8;

class PyObjectWrapper : public node::ObjectWrap
{
public:
    PyObjectWrapper(PyObject* py_object);
    virtual ~PyObjectWrapper();

    static void Initialize();

    static PyObject* ConvertToPython(const Handle<Value>& js_value);
    static Handle<Value> ConvertToJS(PyObject* py_object);

    static PyObject* PythonNamedGetter(PyObject* py_object, const char* key);
    static void PythonNamedSetter(PyObject* py_object, const char* key, PyObject* py_value);
    static PyObject* PythonIndexedGetter(PyObject* py_object, uint32_t index);
    static void PythonIndexedSetter(PyObject* py_object, uint32_t index, PyObject* py_value);

    static Handle<Value> NamedGetter(PyObject* py_object, const char* key);
    static void NamedSetter(PyObject* py_object, const char* key, PyObject* py_value);
    static Handle<Value> IndexedGetter(PyObject* py_object, uint32_t index);
    static void IndexedSetter(PyObject* py_object, uint32_t index, PyObject* py_value);

    static Handle<Value> New(PyObject* py_object);
    static void New(const FunctionCallbackInfo<Value>& js_args);

    static void NamedGetter(Local<String> js_key, const PropertyCallbackInfo<Value>& js_info);
    static void NamedSetter(Local<String> js_key, Local<Value> js_value, const PropertyCallbackInfo<Value>& js_info);
    static void IndexedGetter(uint32_t index, const PropertyCallbackInfo<Value>& js_info);
    static void IndexedSetter(uint32_t index, Local<Value> js_value, const PropertyCallbackInfo<Value>& js_info);
    static void Enumerator(const PropertyCallbackInfo<Array>& js_info);

    static void CallAccessor(Local<String> js_property, const PropertyCallbackInfo<Value>& js_info);
    static void ValueOfAccessor(Local<String> js_property, const PropertyCallbackInfo<Value>& js_info);
    static void ToStringAccessor(Local<String> js_property, const PropertyCallbackInfo<Value>& js_info);

    static void Call(const FunctionCallbackInfo<Value>& js_args);
    static void ValueOf(const FunctionCallbackInfo<Value>& js_args);
    static void ToString(const FunctionCallbackInfo<Value>& js_args);

    PyObject* InstanceGetPyObject();

    Handle<Value> InstanceNamedGetter(Local<String> js_key);
    Handle<Value> InstanceNamedSetter(Local<String> js_key, Local<Value> js_value);
    Handle<Value> InstanceIndexedGetter(uint32_t index);
    Handle<Value> InstanceIndexedSetter(uint32_t index, Local<Value> js_value);
    Handle<Array> InstanceEnumerator();

    Handle<Value> InstanceCall(const FunctionCallbackInfo<Value>& js_args);
    Handle<Value> InstanceValueOf(const FunctionCallbackInfo<Value>& js_args);
    Handle<Value> InstanceToString(const FunctionCallbackInfo<Value>& js_args);

public:
    static Persistent<FunctionTemplate> py_function_template;

private:
    static PyTypeObject PyCFunction_Type_x;

    static void py_method_init_x(void);
    static void py_method_dealloc_x(PyObject* py_object);

    static PyObject* py_method_function_x(PyObject* py_self, PyObject* py_args);

    static void uv_work_create(PyObject* py_object, PyObject* py_args);
    static void uv_work_cb(uv_work_t* req);
    static void uv_after_work_cb(uv_work_t* req, int status);

private:
    PyObject* __py_object;
};

#endif /* PY_OBJECT_WRAPPER_H */
