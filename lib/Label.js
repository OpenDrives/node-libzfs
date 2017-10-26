var libzfs = require('../build/Release/node_libzfs');
var names = require('./Names');
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

Label.prototype.isActive = function(){
	return this.state && this.state != Label.POOL_STATE_DESTROYED;
}


// lifting from the enum in usr/src/uts/common/sys/fs/zfs.h
Label.POOL_STATE_ACTIVE = 0;                /* In active use              */
Label.POOL_STATE_EXPORTED = 1;              /* Explicitly exported        */
Label.POOL_STATE_DESTROYED = 2;             /* Explicitly destroyed       */
Label.POOL_STATE_SPARE = 3;                 /* Reserved for hot spare use */
Label.POOL_STATE_L2CACHE = 4;               /* Level 2 ARC device         */
Label.POOL_STATE_UNINITIALIZED = 5;         /* Internal spa_t state       */
Label.POOL_STATE_UNAVAIL = 6;               /* Internal libzfs state      */
Label.POOL_STATE_POTENTIALLY_ACTIVE = 7;    /* Internal libzfs state      */