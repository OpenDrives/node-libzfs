## Native Node.js connector for libzfs.so

**Very** limited implementation

##### Supports:

* printing device labels

### Download
```bash
git clone https://gitlab.com/opendrives/node-libzfs.git
cd node-libzfs
```

### Install + Build
(requires library `/lib/libzfs.so`)
```bash
MAKE=gmake CC=/opt/gcc/4.4.4/bin/gcc CXX=/opt/gcc/4.4.4/bin/g++ npm install
```

### Try it out

```
open@drkatz:~/node-libzfs$ sudo node bin/index.js /dev/rdsk/c3t15d1s0
* Pool [V02] was DESTROYED
```

```
open@drkatz:~/node-libzfs$ sudo node bin/index.js /dev/rdsk/foobar
Device [/dev/rdsk/foobar] has no label or is not a valid device
```

### Using it in a script

```
var lib = require('libzfs');
var label = lib.getLabel('/dev/rdsk/c3t15d1s0')
console.log(label)
// { name: 'V02', state: 2 }
console.log(lib.getPoolStates()[label['state']]);
// POOL_STATE_DESTROYED
```


