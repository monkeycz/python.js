# node-python binding

python bridge for nodejs!


## Usage

```javascript

// python stuff
var python = require('node-python');
var os = python.import('os');

// nodejs stuff
var path = require('path');

assert(os.path.basename(os.getcwd()) == path.basename(process.cwd()));

```


## Build

### For Mac

#### For Node.js
```bash

node-gyp rebuild --arch=[ia32/x64] --target=x.y.z 
# target nodejs version, eg: 0.10.28

```

#### For node-webkit
```bash

nw-gyp rebuild --arch=[ia32/x64] --target=x.y.z 
# target node-webkit version, eg: 0.8.6

```

### For Windows

#### For Node.js
```bash

cd node-python\src
cl /O2 /MT /LD -Febinding.node binding.cc py_object_wrapper.cc utils.cc <NODE_GYP_ROOT>\.node-gyp\<NODEJS_VERSION>\<NODEJS_ARCH>\node.lib <PYTHON_ROOT>\Python27\libs\python27.lib /I<NODE_GYP_ROOT>\.node-gyp\<NODEJS_VERSION>\src /I<NODE_GYP_ROOT>\.node-gyp\<NODEJS_VERSION>\deps\uv\include /I<NODE_GYP_ROOT>\.node-gyp\<NODEJS_VERSION>\deps\v8\include /I<PYTHON_ROOT>\Python27\include
copy binding.node <NODE_MODULES_ROOT>\node-python\compiled\<NODEJS_VERSION>\win32\<NODEJS_ARCH>

```

#### For node-webkit
```bash

cd node-python\src
cl /O2 /MT /LD -Febinding.node binding.cc py_object_wrapper.cc utils.cc <NW_GYP_ROOT>\.nw-gyp\<NODE_WEBKIT_VERSION>\<NODE_WEBKIT_ARCH>\nw.lib <PYTHON_ROOT>\Python27\libs\python27.lib /I<NW_GYP_ROOT>\.nw-gyp\<NODE_WEBKIT_VERSION>\src /I<NW_GYP_ROOT>\.nw-gyp\<NODE_WEBKIT_VERSION>\deps\uv\include /I<NW_GYP_ROOT>\.nw-gyp\<NODE_WEBKIT_VERSION>\deps\v8\include /I<PYTHON_ROOT>\Python27\include
copy binding.node <NODE_MODULES_ROOT>\node-python\compiled\<NODEJS_VERSION>\win32\<NODE_WEBKIT_ARCH>

```


## Install

```bash

npm install <protocol>://<user>@<domain>/<projects>/node-python.git
# eg: npm install git+ssh://git@example.com/wapp/node-python.git

```


## Test

```bash

node test/jstest.js

```
