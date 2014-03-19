
#ifndef UTILS_H
#define UTILS_H

#include <v8.h>

using namespace v8;

Handle<Value> ThrowPythonException(void);
void ThrowJSException(TryCatch& js_try_catch);

#endif /* UTILS_H */
