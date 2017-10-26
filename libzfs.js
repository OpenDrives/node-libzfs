
var Label = require('./lib/Label');
var libzfs = require('./build/Release/node_libzfs');
var names = require('./lib/Names');

module.exports = {
	getDeviceLabel:function(device_input){
		return new Label(device_input);
	},
	getPoolStatus:function(pool_name){
		var status = libzfs.GetPoolStatus(pool_name);
		if(!status) return undefined;
		status.scan['state_name'] = libzfs.GetEnumValue('scan_state',status.scan.state);
		status.scan['func_name'] = libzfs.GetEnumValue('scan_function',status.scan.function);
		status['status_name'] = libzfs.GetEnumValue('status',status.status);
		return status;
	}

}

module.exports.getLabelStateNames = names.getLabelStateNames();
module.exports.getPoolStatusNames = names.getPoolStatusNames();
module.exports.getScanStateNames = names.getScanStateNames();
module.exports.getPoolScanFunctionNames = names.getPoolScanFunctionNames();
