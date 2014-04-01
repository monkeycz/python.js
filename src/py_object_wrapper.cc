
#include "py_object_wrapper.h"
#include "utils.h"

PyObjectWrapper::PyObjectWrapper(PyObject* py_object) : 
    node::ObjectWrap(), __py_object(py_object)
{
}

PyObjectWrapper::~PyObjectWrapper()
{
    Py_XDECREF(__py_object);
    __py_object = NULL;
}

void PyObjectWrapper::Initialize()
{
    HandleScope scope;

    Local<FunctionTemplate> function_template = FunctionTemplate::New();

    Local<ObjectTemplate> instance_template = function_template->InstanceTemplate();
    instance_template->SetInternalFieldCount(1);
    instance_template->SetNamedPropertyHandler(NamedGetter, NamedSetter, NULL, NULL, Enumerator);
    instance_template->SetIndexedPropertyHandler(IndexedGetter, IndexedSetter, NULL, NULL, Enumerator);
    instance_template->SetCallAsFunctionHandler(Call, Handle<Value>());
    instance_template->SetAccessor(String::NewSymbol("valueOf"), ValueOfAccessor);

    Local<ObjectTemplate> prototype_template = function_template->PrototypeTemplate();
    prototype_template->SetAccessor(String::NewSymbol("toString"), ToStringAccessor);

    py_function_template = Persistent<FunctionTemplate>::New(function_template);

    py_method_init_x();

    uv_work_init();
}

PyObject* PyObjectWrapper::ConvertToPython(const Handle<Value>& js_value)
{
    HandleScope scope;

    if (js_value->IsUndefined()) {
        Py_RETURN_NONE;
    } else if (js_value->IsNull()) {
        Py_RETURN_NONE;
    } else if (js_value->IsInt32()) {
        return PyInt_FromLong(js_value->Int32Value());
    } else if (js_value->IsUint32()) {
        return PyInt_FromLong(js_value->Uint32Value());
    } else if (js_value->IsNumber()) {
        return PyFloat_FromDouble(js_value->NumberValue());
    } else if (js_value->IsBoolean()) {
        return PyBool_FromLong(js_value->BooleanValue());
    } else if (js_value->IsString()) {
        String::Utf8Value js_value_string(js_value->ToString());
        return PyString_FromString(*js_value_string);
    } else if (js_value->IsArray()) {
        Local<Array> js_array = Array::Cast(*js_value);
        int length = js_array->Length();
        PyObject* py_list = PyList_New(length);
        for (int i = 0; i < length; i++) {
            Local<Value> js_item = js_array->Get(i);
            PyList_SET_ITEM(py_list, i, ConvertToPython(js_item));
        }
        return py_list;
    } else if (js_value->IsFunction()) {
        Local<Function> _js_function = Function::Cast(*js_value);
        Persistent<Function> js_function = Persistent<Function>::New(_js_function);

        PyMethodDef* py_method = (PyMethodDef*)malloc(sizeof(PyMethodDef));
        String::Utf8Value js_function_name_string(js_function->GetName()->ToString());
        py_method->ml_name = strdup(*js_function_name_string);
        py_method->ml_meth = (PyCFunction)py_method_function_x;
        py_method->ml_flags = METH_VARARGS;
        py_method->ml_doc = strdup("");

        PyObject* py_self = PyCObject_FromVoidPtr(*js_function, NULL);

        PyObject* py_function = PyCFunction_New(py_method, py_self);
        Py_TYPE(py_function) = &PyCFunction_Type_x;

        return py_function;
    } else if (js_value->IsNativeError()) {
        return ConvertToPythonException(js_value);
    } else if (js_value->IsObject()) {
        Local<Object> js_object = js_value->ToObject();

        if (!js_object->FindInstanceInPrototypeChain(py_function_template).IsEmpty()) {
            PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_object);
            PyObject* py_object = py_object_wrapper->InstanceGetPyObject();
            Py_XINCREF(py_object);
            return py_object;
        } else {
            PyObject* py_dict = PyDict_New();
            Local<Array> js_property_names = js_object->GetPropertyNames();
            int length = js_property_names->Length();
            for(int i = 0; i < length; i++) {
                Local<Value> js_property_key = js_property_names->Get(i);
                Local<Value> js_property_value = js_object->Get(js_property_key);
                PyObject* py_property_key = ConvertToPython(js_property_key);
                PyObject* py_property_value = ConvertToPython(js_property_value);
                PyDict_SetItem(py_dict, py_property_key, py_property_value);
                Py_XDECREF(py_property_key);
                Py_XDECREF(py_property_value);
            }
            return py_dict;
        }
    }

    Py_RETURN_NONE;
}

Handle<Value> PyObjectWrapper::ConvertToJS(PyObject* py_object)
{
    HandleScope scope;

    if (PyCallable_Check(py_object) != 0) {
        Py_XINCREF(py_object);
        Local<FunctionTemplate> js_function_template = FunctionTemplate::New(Call, New(py_object));
        Local<Function> js_function = js_function_template->GetFunction();
        Handle<Value> js_function_name = NamedGetter(py_object, "func_name");
        if (!js_function_name.IsEmpty())
            js_function->SetName(js_function_name->ToString());
        return scope.Close(js_function);
    } else if (PyExceptionInstance_Check(py_object)) {
        return scope.Close(ConvertToJSException(py_object));
    } else if (PyMapping_Check(py_object) != 0) {
        int length = (int)PyMapping_Length(py_object);
        Local<Object> js_object = Object::New();
        PyObject* py_keys = PyMapping_Keys(py_object);
        PyObject* py_values = PyMapping_Values(py_object);
        for(int i = 0; i < length; i++) {
            PyObject* py_key = PySequence_GetItem(py_keys, i);
            PyObject* py_value = PySequence_GetItem(py_values, i);
            Handle<Value> js_key = ConvertToJS(py_key);
            Handle<Value> js_value = ConvertToJS(py_value);
            Py_XDECREF(py_key);
            Py_XDECREF(py_value);
            js_object->Set(js_key, js_value);
        }
        Py_XDECREF(py_keys);
        Py_XDECREF(py_values);
        return scope.Close(js_object);
    } else if (PyList_Check(py_object) || PyTuple_Check(py_object)) {
        int length = (int)PySequence_Length(py_object);
        Local<Array> js_array = Array::New(length);
        for(int i = 0; i < length; i++) {
            PyObject* py_item = PySequence_GetItem(py_object, i);
            Handle<Value> js_item = ConvertToJS(py_item);
            Py_XDECREF(py_item);
            js_array->Set(i, js_item);
        }
        return scope.Close(js_array);
    } else {
        Py_XINCREF(py_object);
        Handle<Value> js_object = New(py_object);
        return scope.Close(js_object);
    }

    return Undefined();
}

PyObject* PyObjectWrapper::PythonNamedGetter(PyObject* py_object, const char* key)
{
    PyObject* py_key = PyString_FromString(key);
    PyObject* py_value = NULL;

    if (PyMapping_HasKey(py_object, py_key) != 0)
        py_value = PyObject_GetItem(py_object, py_key);
    else if (PyObject_HasAttr(py_object, py_key) != 0)
        py_value = PyObject_GetAttr(py_object, py_key);

    Py_XDECREF(py_key);

    return py_value;
}

void PyObjectWrapper::PythonNamedSetter(PyObject* py_object, const char* key, PyObject* py_value)
{
    PyObject* py_key = PyString_FromString(key);

    if (PyMapping_Check(py_object) != 0)
        PyObject_SetItem(py_object, py_key, py_value);
    else
        PyObject_SetAttr(py_object, py_key, py_value);

    Py_XDECREF(py_key);
}

PyObject* PyObjectWrapper::PythonIndexedGetter(PyObject* py_object, uint32_t index)
{
    if (PySequence_Check(py_object) != 0 && index < (uint32_t)PySequence_Size(py_object)) {
        return PySequence_GetItem(py_object, index);
    } else {
        return NULL;
    }
}

void PyObjectWrapper::PythonIndexedSetter(PyObject* py_object, uint32_t index, PyObject* py_value)
{
    if (PySequence_Check(py_object) != 0 && index < (uint32_t)PySequence_Size(py_object)) {
        PySequence_SetItem(py_object, index, py_value);
    }
}

Handle<Value> PyObjectWrapper::NamedGetter(PyObject* py_object, const char* key)
{
    HandleScope scope;
    PyObject* py_value = PythonNamedGetter(py_object, key);
    if (py_value != NULL)
        return scope.Close(New(py_value));
    else
        return Handle<Value>();
}

void PyObjectWrapper::NamedSetter(PyObject* py_object, const char* key, PyObject* py_value)
{
    HandleScope scope;
    PythonNamedSetter(py_object, key, py_value);
}

Handle<Value> PyObjectWrapper::IndexedGetter(PyObject* py_object, uint32_t index)
{
    HandleScope scope;
    PyObject* py_item = PythonIndexedGetter(py_object, index);
    if (py_item != NULL)
        return scope.Close(New(py_item));
    else
        return Handle<Value>();
}

void PyObjectWrapper::IndexedSetter(PyObject* py_object, uint32_t index, PyObject* py_value)
{
    HandleScope scope;
    PythonIndexedSetter(py_object, index, py_value);
}

Handle<Value> PyObjectWrapper::New(PyObject* py_object)
{
    HandleScope scope;

    Local<Value> js_value;
    if (py_object == Py_None) {
        js_value = Local<Value>::New(Null());
    } else if (PyFloat_CheckExact(py_object)) {
        double value = PyFloat_AsDouble(py_object);
        js_value = Local<Value>::New(Number::New(value));
    } else if (PyInt_CheckExact(py_object)) {
        long value = PyInt_AsLong(py_object);
        js_value = Local<Value>::New(Integer::New((int32_t)value));
    } else if (PyLong_CheckExact(py_object)) {
        long value = PyLong_AsLong(py_object);
        js_value = Local<Value>::New(Integer::New((int32_t)value));
    } else if (PyString_CheckExact(py_object)) {
        char* value = PyString_AsString(py_object);
        if (value != NULL)
            js_value = Local<Value>::New(String::New(value));
    } else if (PyBool_Check(py_object)) {
        int value = PyObject_IsTrue(py_object);
        if (value != -1)
            js_value = Local<Value>::New(Boolean::New(value));
    }

    if (PyErr_Occurred()) {
        Py_XDECREF(py_object);
        return ThrowPythonException();
    }

    if (js_value.IsEmpty()) {
        Local<Object> js_object = py_function_template->GetFunction()->NewInstance();
        PyObjectWrapper* py_object_wrapper = new PyObjectWrapper(py_object);
        py_object_wrapper->Wrap(js_object);
        js_value = Local<Value>::New(js_object);
    } else {
        Py_XDECREF(py_object);
    }

    return scope.Close(js_value);
}

Handle<Value> PyObjectWrapper::NamedGetter(Local<String> js_key, const AccessorInfo& js_info)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_info.Holder());
    Handle<Value> js_result = py_object_wrapper->InstanceNamedGetter(js_key);
    return scope.Close(js_result);
}

Handle<Value> PyObjectWrapper::NamedSetter(Local<String> js_key, Local<Value> js_value, const AccessorInfo& js_info)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_info.Holder());
    Handle<Value> js_result = py_object_wrapper->InstanceNamedSetter(js_key, js_value);
    return scope.Close(js_result);
}

Handle<Array> PyObjectWrapper::Enumerator(const AccessorInfo& js_info)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_info.Holder());
    Handle<Array> js_result = py_object_wrapper->InstanceEnumerator();
    return scope.Close(js_result);
}

Handle<Value> PyObjectWrapper::IndexedGetter(uint32_t index, const AccessorInfo& js_info)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_info.Holder());
    Handle<Value> js_result = py_object_wrapper->InstanceIndexedGetter(index);
    return scope.Close(js_result);
}

Handle<Value> PyObjectWrapper::IndexedSetter(uint32_t index, Local<Value> js_value, const AccessorInfo& js_info)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_info.Holder());
    Handle<Value> js_result = py_object_wrapper->InstanceIndexedSetter(index, js_value);
    return scope.Close(js_result);
}

Handle<Value> PyObjectWrapper::CallAccessor(Local<String> js_property, const AccessorInfo& js_info)
{
    HandleScope scope;
    Local<FunctionTemplate> js_function = FunctionTemplate::New(Call);
    return scope.Close(js_function->GetFunction());
}

Handle<Value> PyObjectWrapper::ToStringAccessor(Local<String> js_property, const AccessorInfo& js_info)
{
    HandleScope scope;
    Local<FunctionTemplate> js_function = FunctionTemplate::New(ToString);
    return scope.Close(js_function->GetFunction());
}

Handle<Value> PyObjectWrapper::ValueOfAccessor(Local<String> js_property, const AccessorInfo& js_info)
{
    HandleScope scope;
    Local<FunctionTemplate> js_function = FunctionTemplate::New(ValueOf);
    return scope.Close(js_function->GetFunction());
}

Handle<Value> PyObjectWrapper::Call(const Arguments& js_args)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = NULL;
    if (js_args.Data()->IsUndefined())
        py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_args.This());
    else
        py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_args.Data()->ToObject());
    Handle<Value> js_result = py_object_wrapper->InstanceCall(js_args);
    return scope.Close(js_result);
}

Handle<Value> PyObjectWrapper::ToString(const Arguments& js_args)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_args.This());
    Handle<Value> js_string = py_object_wrapper->InstanceToString(js_args);
    return scope.Close(js_string);
}

Handle<Value> PyObjectWrapper::ValueOf(const Arguments& js_args)
{
    HandleScope scope;
    PyObjectWrapper* py_object_wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(js_args.This());
    Handle<Value> js_value = py_object_wrapper->InstanceValueOf(js_args);
    return scope.Close(js_value);
}

PyObject* PyObjectWrapper::InstanceGetPyObject()
{
    return __py_object;
}

Handle<Value> PyObjectWrapper::InstanceNamedGetter(Local<String> js_key)
{
    HandleScope scope;
    PyObject* py_object = InstanceGetPyObject();
    String::Utf8Value key(js_key);
    return scope.Close(NamedGetter(py_object, *key));
}

Handle<Value> PyObjectWrapper::InstanceNamedSetter(Local<String> js_key, Local<Value> js_value)
{
    HandleScope scope;
    PyObject* py_object = InstanceGetPyObject();
    String::Utf8Value key(js_key);
    PyObject* py_value = ConvertToPython(js_value);
    NamedSetter(py_object, *key, py_value);
    Py_XDECREF(py_value);
    return scope.Close(js_value);
}

Handle<Value> PyObjectWrapper::InstanceIndexedGetter(uint32_t index)
{
    HandleScope scope;
    PyObject* py_object = InstanceGetPyObject();
    return scope.Close(IndexedGetter(py_object, index));
}

Handle<Value> PyObjectWrapper::InstanceIndexedSetter(uint32_t index, Local<Value> js_value)
{
    HandleScope scope;
    PyObject* py_object = InstanceGetPyObject();
    PyObject* py_value = ConvertToPython(js_value);
    IndexedSetter(py_object, index, py_value);
    Py_XDECREF(py_value);
    return scope.Close(js_value);
}

Handle<Array> PyObjectWrapper::InstanceEnumerator()
{
    HandleScope scope;

    PyObject* py_object = InstanceGetPyObject();

    PyObject* py_dir = PyObject_Dir(py_object);
    Local<Array> js_dir = Array::Cast(*ConvertToJS(py_dir));
    Py_XDECREF(py_dir);

    return scope.Close(js_dir);
}

Handle<Value> PyObjectWrapper::InstanceCall(const Arguments& js_args)
{
    HandleScope scope;

    PyObject* py_object = InstanceGetPyObject();

    int length = js_args.Length();
    PyObject* py_args = PyTuple_New(length);
    for (int i = 0; i < length; i++) {
        PyObject* py_arg = ConvertToPython(js_args[i]);
        PyTuple_SET_ITEM(py_args, i, py_arg);
    }

    Handle<Value> js_async = NamedGetter(py_object, "async");
    if (!js_async.IsEmpty() && js_async->ToBoolean()->Value()) {
        uv_work_create(py_object, py_args);
        Py_XDECREF(py_args);
        return Undefined();
    }

    PyObject* py_result = PyObject_CallObject(py_object, py_args);

    Py_XDECREF(py_args);

    if (py_result != NULL)
        return scope.Close(New(py_result));
    else
        return ThrowPythonException();
}

Handle<Value> PyObjectWrapper::InstanceValueOf(const Arguments& js_args)
{
    HandleScope scope;
    PyObject* py_object = InstanceGetPyObject();
    return scope.Close(ConvertToJS(py_object));
}

Handle<Value> PyObjectWrapper::InstanceToString(const Arguments& js_args)
{
    HandleScope scope;

    PyObject* py_object = InstanceGetPyObject();

    PyObject* py_string = PyObject_Str(py_object);
    if (py_string == NULL)
        return ThrowPythonException();

    Local<String> js_string = String::New(PyString_AsString(py_string));

    Py_XDECREF(py_string);

    return scope.Close(js_string);
}

Persistent<FunctionTemplate> PyObjectWrapper::py_function_template;

PyTypeObject PyObjectWrapper::PyCFunction_Type_x;

void PyObjectWrapper::py_method_init_x(void)
{
    PyCFunction_Type_x = PyCFunction_Type;
    PyCFunction_Type_x.tp_dealloc = py_method_dealloc_x;
}

void PyObjectWrapper::py_method_dealloc_x(PyObject* py_object)
{
    HandleScope scope;

    PyCFunctionObject* py_function_object = (PyCFunctionObject*)py_object;

    PyMethodDef* py_method = py_function_object->m_ml;
    free((void*)py_method->ml_name);
    free((void*)py_method->ml_doc);
    free(py_method);

    PyObject* py_self = py_function_object->m_self;
    Persistent<Function> js_function = (Function*)PyCObject_AsVoidPtr(py_self);
    js_function.Dispose();

    PyCFunction_Type.tp_dealloc(py_object);
}

PyObject* PyObjectWrapper::py_method_function_x(PyObject* py_self, PyObject* py_args)
{
    HandleScope scope;

    int js_argc = (int)PySequence_Length(py_args);
    Handle<Value>* js_argv = new Handle<Value>[js_argc];
    for (int i = 0; i < js_argc; i++) {
        PyObject* py_item = PySequence_GetItem(py_args, i);
        js_argv[i] = New(py_item);
    }

    Persistent<Function> js_function = (Function*)PyCObject_AsVoidPtr(py_self);

    TryCatch js_try_catch;
    Local<Value> js_result = js_function->Call(Context::GetCurrent()->Global(), js_argc, js_argv);

    delete[] js_argv;

    if (!js_result.IsEmpty())
        return ConvertToPython(js_result);
    else
        return ThrowJSException(js_try_catch);
}

typedef struct uv_work_data {
    PyObject* py_object;
    PyObject* py_args;
    PyObject* py_result;

    bool has_caught_exception;
    PyObject* py_exception_type;
    PyObject* py_exception_value;
    PyObject* py_exception_traceback;

    PyObject* py_async_cb;
} uv_work_data_t;

uv_mutex_t PyObjectWrapper::uv_mutex;

void PyObjectWrapper::uv_work_init(void)
{
    uv_mutex_init(&uv_mutex);
}

void PyObjectWrapper::uv_work_create(PyObject* py_object, PyObject* py_args)
{
    uv_work_data_t* data = (uv_work_data_t*)malloc(sizeof(uv_work_data_t));
    Py_XINCREF(py_object);
    data->py_object = py_object;
    Py_XINCREF(py_args);
    data->py_args = py_args;
    data->py_result = NULL;
    data->has_caught_exception = false;
    data->py_exception_type = NULL;
    data->py_exception_value = NULL;
    data->py_exception_traceback = NULL;
    data->py_async_cb = PythonNamedGetter(py_object, "async_cb");

    uv_work_t* req = (uv_work_t*)malloc(sizeof(uv_work_t));
    req->data = data;

    uv_queue_work(uv_default_loop(), req, uv_work_cb, uv_after_work_cb);
}

void PyObjectWrapper::uv_work_cb(uv_work_t* req)
{
    uv_work_data_t* data = (uv_work_data_t*)req->data;

    uv_mutex_lock(&uv_mutex);

    data->py_result = PyObject_CallObject(data->py_object, data->py_args);

    if (data->py_result == NULL) {
        data->has_caught_exception = CatchPythonException(&data->py_exception_type, 
            &data->py_exception_value, &data->py_exception_traceback);
    }

    uv_mutex_unlock(&uv_mutex);

    Py_XDECREF(data->py_args);
    Py_XDECREF(data->py_object);
}

void PyObjectWrapper::uv_after_work_cb(uv_work_t* req, int status)
{
    HandleScope scope;

    assert(status == 0);

    uv_work_data_t* data = (uv_work_data_t*)req->data;

    uv_mutex_lock(&uv_mutex);

    int js_argc = 2;
    Handle<Value> js_argv[2];

    if (data->py_result != NULL) {
        js_argv[0] = Local<Value>::New(Null());
        js_argv[1] = New(data->py_result);
    } else {
        Handle<Value> js_exception;
        if (data->has_caught_exception) {
            js_exception = ConvertToJSException(data->py_exception_type, 
                data->py_exception_value, data->py_exception_traceback);
            ReleasePythonException(data->py_exception_type, 
                data->py_exception_value, data->py_exception_traceback);
        } else {
            js_exception = Null();
        }

        js_argv[0] = Local<Value>::New(js_exception);
        js_argv[1] = Local<Value>::New(Null());
    }

    if (data->py_async_cb != NULL) {
        Handle<Value> js_async_cb = ConvertToJS(data->py_async_cb);
        Py_XDECREF(data->py_async_cb);

        if (!js_async_cb.IsEmpty() && js_async_cb->IsFunction()) {
            Function* js_async_cb_function = Function::Cast(*js_async_cb);
            js_async_cb_function->Call(Context::GetCurrent()->Global(), js_argc, js_argv);
        }
    }

    uv_mutex_unlock(&uv_mutex);

    free(data);
    free(req);
}
