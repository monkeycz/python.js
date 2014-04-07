
#ifndef PY_OBJECT_WRAPPER_H
#define PY_OBJECT_WRAPPER_H

#include <node.h>
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
    Handle<Value> InstanceToString(const Arguments& js_args);

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
