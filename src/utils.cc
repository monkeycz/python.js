
#include "utils.h"

void PyInit(void)
{
    Py_Initialize();
    PyEval_InitThreads();
    PyEval_ReleaseThread(PyThreadState_Get());
}

void PyExit(void* arg)
{
    PyGILState_Ensure();
    Py_Finalize();
}

bool CatchPythonException(PyObject** py_exception_type, PyObject** py_exception_value, PyObject** py_exception_traceback)
{
    PyThreadStateLock py_thread_lock;

    PyErr_Fetch(py_exception_type, py_exception_value, py_exception_traceback);
    return *py_exception_type != NULL;
}

void ReleasePythonException(PyObject* py_exception_type, PyObject* py_exception_value, PyObject* py_exception_traceback)
{
    PyThreadStateLock py_thread_lock;

    Py_XDECREF(py_exception_type);
    Py_XDECREF(py_exception_value);
    Py_XDECREF(py_exception_traceback);
}

Handle<Value> ConvertToJSException(PyObject* py_exception_type, PyObject* py_exception_value, PyObject* py_exception_traceback)
{
    PyThreadStateLock py_thread_lock;

    EscapableHandleScope scope(Isolate::GetCurrent());

    if (py_exception_type == NULL)
        return scope.Escape(Exception::Error(String::NewFromUtf8(Isolate::GetCurrent(), "No exception found")));

    Local<String> js_message;
    if (py_exception_value != NULL) {
        if (PyObject_TypeCheck(py_exception_value, &PyString_Type)) {
            js_message = String::NewFromUtf8(Isolate::GetCurrent(), PyString_AsString(py_exception_value));
        } else if (PyObject_TypeCheck(py_exception_value, &PyTuple_Type) && 
            PyTuple_Size(py_exception_value) > 0) {
            js_message = String::NewFromUtf8(Isolate::GetCurrent(), PyString_AsString(PyTuple_GetItem(py_exception_value, 0)));
        } else {
            PyObject* py_exception_value_string = PyObject_Str(py_exception_value);
            js_message = String::NewFromUtf8(Isolate::GetCurrent(), PyString_AsString(py_exception_value_string));
            Py_XDECREF(py_exception_value_string);            
        }
    } else {
        js_message = String::NewFromUtf8(Isolate::GetCurrent(), "Unknown exception");
    }

    Local<Value> js_exception;
    if (PyErr_GivenExceptionMatches(py_exception_type, PyExc_IndexError) != 0) {
        js_exception = Exception::RangeError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_exception_type, PyExc_ReferenceError) != 0) {
        js_exception = Exception::ReferenceError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_exception_type, PyExc_SyntaxError) != 0) {
        js_exception = Exception::SyntaxError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_exception_type, PyExc_TypeError) != 0) {
        js_exception = Exception::TypeError(js_message);
    } else {
        js_exception = Exception::Error(js_message);
    }

    return scope.Escape(js_exception);
}

Handle<Value> ConvertToJSException(PyObject* py_exception)
{
    EscapableHandleScope scope(Isolate::GetCurrent());
    Local<Value> js_exception = ConvertToJSException(py_exception, py_exception, NULL);
    return scope.Escape(js_exception);
}

Handle<Value> ThrowPythonException(void)
{
    PyThreadStateLock py_thread_lock;

    HandleScope scope(Isolate::GetCurrent());

    PyObject* py_exception_type = NULL;
    PyObject* py_exception_value = NULL;
    PyObject* py_exception_traceback = NULL;

    if (!CatchPythonException(&py_exception_type, &py_exception_value, &py_exception_traceback))
        return Null(Isolate::GetCurrent());

    Handle<Value> js_exception = ConvertToJSException(py_exception_type, py_exception_value, py_exception_traceback);

    // if (py_exception_traceback != NULL) {
    //     PyObject* py_exception_traceback_string = PyObject_Str(py_exception_traceback);
    //     printf("%s\n", PyString_AsString(py_exception_traceback_string));
    //     Py_XDECREF(py_exception_traceback_string);
    // }

    ReleasePythonException(py_exception_type, py_exception_value, py_exception_traceback);

    return Isolate::GetCurrent()->ThrowException(js_exception);
}

Handle<Value> CatchJSException(TryCatch& js_try_catch)
{
    EscapableHandleScope scope(Isolate::GetCurrent());

    if (!js_try_catch.HasCaught())
        return Handle<Value>();

    Local<Value> js_exception = js_try_catch.Exception();

    return scope.Escape(js_exception);
}

PyObject* ConvertToPythonException(Handle<Value> js_exception)
{
    PyThreadStateLock py_thread_lock;

    HandleScope scope(Isolate::GetCurrent());

    PyObject* py_exception_type = NULL;
    Local<Value> js_message;

    if (js_exception->IsObject()) {
        Local<Object> js_exception_object = js_exception->ToObject();

        String::Utf8Value js_exception_name_string(js_exception_object->Get(String::NewFromUtf8(Isolate::GetCurrent(), "name")));
        char* exception_name = *js_exception_name_string;
        if (strcmp(exception_name, "RangeError") == 0) {
            py_exception_type = PyExc_IndexError;
        } else if (strcmp(exception_name, "ReferenceError") == 0) {
            py_exception_type = PyExc_ReferenceError;
        } else if (strcmp(exception_name, "SyntaxError") == 0) {
            py_exception_type = PyExc_SyntaxError;
        } else if (strcmp(exception_name, "TypeError") == 0) {
            py_exception_type = PyExc_TypeError;
        } else if (strcmp(exception_name, "Error") == 0) {
            py_exception_type = PyExc_Exception;
        } else {
            py_exception_type = PyExc_Exception;
        }
        js_message = js_exception_object->Get(String::NewFromUtf8(Isolate::GetCurrent(), "message"));
    } else {
        Local<String> js_exception_string = js_exception->ToString();

        py_exception_type = PyExc_Exception;
        js_message = js_exception_string;
    }

    PyObject* py_args = PyTuple_New(1);
    String::Utf8Value js_message_string(js_message);
    PyObject* py_arg = PyString_FromString(*js_message_string);
    PyTuple_SET_ITEM(py_args, 0, py_arg);

    PyObject* py_exception = PyObject_CallObject(py_exception_type, py_args);

    Py_XDECREF(py_args);

    return py_exception;
}

PyObject* ThrowJSException(TryCatch& js_try_catch)
{
    PyThreadStateLock py_thread_lock;

    HandleScope scope(Isolate::GetCurrent());

    Handle<Value> js_exception = CatchJSException(js_try_catch);
    if (js_exception.IsEmpty())
        return NULL;

    PyObject* py_exception = ConvertToPythonException(js_exception);
    PyErr_SetObject(py_exception, py_exception);
    Py_XDECREF(py_exception);

    // String::Utf8Value js_exception_stacktrace_string(js_try_catch.StackTrace());
    // printf("%s\n", *js_exception_stacktrace_string);

    return NULL;
}
