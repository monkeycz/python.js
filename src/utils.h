
#ifndef UTILS_H
#define UTILS_H

#include <node.h>
#include <Python.h>

using namespace v8;

class PyThreadStateLock
{
public:
    PyThreadStateLock(void)
    {
        py_gil_state = PyGILState_Ensure();
    }

    ~PyThreadStateLock(void)
    {
        PyGILState_Release(py_gil_state);
    }

private:
    PyGILState_STATE py_gil_state;
};

void PyInit(void);
void PyExit(void* arg);

bool CatchPythonException(PyObject** py_exception_type, PyObject** py_exception_value, PyObject** py_exception_traceback);
void ReleasePythonException(PyObject* py_exception_type, PyObject* py_exception_value, PyObject* py_exception_traceback);
Handle<Value> ConvertToJSException(PyObject* py_exception_type, PyObject* py_exception_value, PyObject* py_exception_traceback);
Handle<Value> ConvertToJSException(PyObject* py_exception);
Handle<Value> ThrowPythonException(void);

Handle<Value> CatchJSException(TryCatch& js_try_catch);
PyObject* ConvertToPythonException(Handle<Value> js_exception);
PyObject* ThrowJSException(TryCatch& js_try_catch);

#endif /* UTILS_H */
