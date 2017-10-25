#!/usr/bin/env node
var device_name = process.argv[2];
var libzfs = require("../");
console.log(JSON.stringify(libzfs.getDeviceLabel(device_name),null,4));
