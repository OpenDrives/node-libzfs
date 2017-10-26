## Native Node.js connector for libzfs.so

**Very** limited implementation
##### Supports:

* printing device labels
* json output of zpool status

### Install Globally

```bash
npm install -g node-libzfs
```

get status

```bash
$ node-zpool rpool
{
    "name": "rpool",
    "guid": "18324883825748112812",
    "state": "ONLINE",
    "status": 19,
    "scan": {
        "state": 2,
        "start": 1508960008,
        "end": 1508961231,
        "function": 1,
        "examined": 164869386240,
        "examined_hr": "154G",
        "to_examine": 164862693376,
        "to_examine_hr": "154G",
        "total": 164862693376,
        "total_hr": "154G",
        "fraction_done": 1.0000405965950387,
        "state_name": "FINISHED",
        "func_name": "SCRUB"
    },
    "config": {
        "name": "rpool",
        "type": "root",
        "state": "ONLINE",
        "read_errors": "0",
        "write_errors": "0",
        "checksum_errors": "0",
        "children": [
            {
                "physpath": "/pci@0,0/pci8086,2f02@1/pci15d9,809@0/sd@c,1:a",
                "path": "/dev/dsk/c3t12d1s0",
                "name": "c3t12d1s0",
                "type": "disk",
                "state": "ONLINE",
                "read_errors": "0",
                "write_errors": "0",
                "checksum_errors": "0",
                "children": []
            }
        ]
    }
}
```



## Labels

### Try it out

```
node-zlabels /dev/dsk/c3t15d1s0
{
    "name": "V01",
    "state": 0,
    "conf_guid": "11834331267048583435",
    "pool_guid": "15848552477060795772",
    "state_name": "POOL_ACTIVE"
}
```


### Using it in a script

```
var lib = require('libzfs');
var label = lib.getLabel('/dev/rdsk/c3t15d1s0')
console.log(label)
//{
//     "name": "V01",
//     "state": 0,
//     "conf_guid": "11834331267048583435",
//     "pool_guid": "15848552477060795772",
//     "state_name": "POOL_ACTIVE"
// }
```


