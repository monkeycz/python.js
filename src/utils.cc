
#include <Python.h>
#include <pyerrors.h>

#include "utils.h"

Handle<Value> ThrowPythonException(void)
{
    HandleScope scope;

    PyObject* py_type = NULL;
    PyObject* py_value = NULL;
    PyObject* py_traceback = NULL;
    PyErr_Fetch(&py_type, &py_value, &py_traceback);

    if (py_type == NULL) {
        return ThrowException(
            Exception::Error(String::New("No exception found")));
    }

    Local<String> js_message;
    if (py_value != NULL) {
        if (PyObject_TypeCheck(py_value, &PyString_Type)) {
            js_message = String::New(PyString_AsString(py_value));
        } else if (PyObject_TypeCheck(py_value, &PyTuple_Type) && 
            PyTuple_Size(py_value) > 0) {
            js_message = String::New(PyString_AsString(PyTuple_GetItem(py_value, 0)));
        } else {
            PyObject* py_value_string = PyObject_Str(py_value);
            js_message = String::New(PyString_AsString(py_value_string));
            Py_XDECREF(py_value_string);            
        }
    } else {
        js_message = String::New("Unknown exception");
    }

    Local<Value> js_error;
    if (PyErr_GivenExceptionMatches(py_type, PyExc_IndexError) != 0) {
        js_error = Exception::RangeError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_type, PyExc_ReferenceError) != 0) {
        js_error = Exception::ReferenceError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_type, PyExc_SyntaxError) != 0) {
        js_error = Exception::SyntaxError(js_message);
    } else if (PyErr_GivenExceptionMatches(py_type, PyExc_TypeError) != 0) {
        js_error = Exception::TypeError(js_message);
    } else {
        js_error = Exception::Error(js_message);
    }

    // PyObject* py_traceback_str = PyObject_Str(py_traceback);
    // printf("%s\n", PyString_AsString(py_traceback_str));
    // Py_XDECREF(py_traceback_str);

    Py_XDECREF(py_type);
    Py_XDECREF(py_value);
    Py_XDECREF(py_traceback);

    return ThrowException(js_error);
}

void ThrowJSException(TryCatch& js_try_catch)
{
    HandleScope scope;

    if (!js_try_catch.HasCaught())
        return;

    Local<Value> js_exception = js_try_catch.Exception();
    if (js_exception->IsObject()) {
        String::Utf8Value js_name_string(js_exception->ToObject()->Get(String::New("name")));
        String::Utf8Value js_message_string(js_exception->ToObject()->Get(String::New("message")));

        char* name = *js_name_string;
        char* message = *js_message_string;
        if (strcmp(name, "RangeError") == 0) {
            PyErr_SetString(PyExc_IndexError, message);
        } else if (strcmp(name, "ReferenceError") == 0) {
            PyErr_SetString(PyExc_ReferenceError, message);
        } else if (strcmp(name, "SyntaxError") == 0) {
            PyErr_SetString(PyExc_SyntaxError, message);
        } else if (strcmp(name, "TypeError") == 0) {
            PyErr_SetString(PyExc_TypeError, message);
        } else if (strcmp(name, "Error") == 0) {
            PyErr_SetString(PyExc_Exception, message);
        } else {
            PyErr_SetString(PyExc_Exception, message);
        }
    } else {
        String::Utf8Value js_exception_string(js_exception->ToString());

        char* exception = *js_exception_string;
        PyErr_SetString(PyExc_Exception, exception);
    }

    // String::Utf8Value js_trace_string(js_try_catch.StackTrace());
    // printf("%s\n", *js_trace_string);
}
