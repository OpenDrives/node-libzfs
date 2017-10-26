var libzfs = require('../build/Release/node_libzfs');
function Label(device_input){
	var _l = libzfs.ReadLabel(device_input);
	if(_l){
		this.name = _l.name;
		this.state= _l.state;
		this.conf_guid = _l.conf_guid;
		this.pool_guid = _l.pool_guid;
		this.state_name= libzfs.GetEnumValue('pool_state',_l.state);
	}
}

module.exports = Label;
