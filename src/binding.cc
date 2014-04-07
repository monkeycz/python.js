
#include <node.h>
#include <Python.h>

#include "py_object_wrapper.h"
#include "utils.h"

using namespace v8;
using namespace node;

Handle<Value> import(const Arguments& args)
{
    PyThreadStateLock py_thread_lock;

    HandleScope scope;

    if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
            Exception::Error(String::New("I don't know how to import that.")));
    }

    String::Utf8Value js_module_name_string(args[0]->ToString());
    PyObject* py_module_name = PyString_FromString(*js_module_name_string);

    PyObject* py_module = PyImport_Import(py_module_name);

    Py_XDECREF(py_module_name);

    if (py_module != NULL)
        return scope.Close(PyObjectWrapper::New(py_module));
    else
        return ThrowPythonException();
}

void init(Handle<Object> exports)
{
    PyInit();
    AtExit(PyExit, NULL);

    HandleScope scope;

    PyObjectWrapper::Initialize();

    // module.exports.import
    exports->Set(String::NewSymbol("import"),
        FunctionTemplate::New(import)->GetFunction());

    // module.exports.PyObject
    exports->Set(String::NewSymbol("PyObject"),
        PyObjectWrapper::py_function_template->GetFunction());
}

NODE_MODULE(binding, init)
