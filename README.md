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

