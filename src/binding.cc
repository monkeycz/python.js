
#include <node.h>
#include <Python.h>

#include "py_object_wrapper.h"
#include "utils.h"

using namespace v8;
using namespace node;

void import(const FunctionCallbackInfo<Value>& args)
{
    PyThreadStateLock py_thread_lock;

    HandleScope scope(args.GetIsolate());

    if (args.Length() < 1 || !args[0]->IsString()) {
        args.GetReturnValue().Set(ThrowException(
            Exception::Error(String::New("I don't know how to import that."))));
        return;
    }

    String::Utf8Value js_module_name_string(args[0]->ToString());
    PyObject* py_module_name = PyString_FromString(*js_module_name_string);

    PyObject* py_module = PyImport_Import(py_module_name);

    Py_XDECREF(py_module_name);

    if (py_module != NULL)
        args.GetReturnValue().Set(PyObjectWrapper::New(py_module));
    else
        args.GetReturnValue().Set(ThrowPythonException());
}

void init(Handle<Object> exports)
{
    PyInit();
    AtExit(PyExit, NULL);

    HandleScope scope(Isolate::GetCurrent());

    PyObjectWrapper::Initialize();

    // module.exports.import
    exports->Set(String::NewSymbol("import"),
        FunctionTemplate::New(import)->GetFunction());

    // module.exports.PyObject
    exports->Set(String::NewSymbol("PyObject"),
        PyObjectWrapper::py_function_template->GetFunction());
}

NODE_MODULE(binding, init)
