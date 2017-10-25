#!/usr/bin/env node
var pool_name = process.argv[2];
var libzfs = require("../");
console.log(JSON.stringify(libzfs.getPoolStatus(pool_name),null,4));
