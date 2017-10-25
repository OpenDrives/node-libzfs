## Native Node.js connector for libzfs.so

**Very** limited implementation

##### Supports:

* printing device labels

### Download
```bash
git clone https://github.com/OpenDrives/node-libzfs
cd node-libzfs
```

### Try it out

```
~/node-libzfs$ sudo node bin/index.js /dev/rdsk/c3t15d1s0
* Pool [V02] was DESTROYED
```

```
~/node-libzfs$ sudo node bin/node-zpool.js /dev/rdsk/foobar
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


