var assert = require('assert');
var libzfs = require('../');

describe('get device Label', function(){

  it('can load libzfs', function(){
    assert(libzfs != null);
  });

  it('find a device with rpool label', function(done){
  	// list devices		

    	var rpool = libzfs.getPoolStatus('rpool');

    	var children = rpool['config']['children'];
    	var child = null;
    	while(!child){
    		if(children[0]['type'] == 'disk'){
    			child = children[0];
    		}else{
    			children = children[0]['children'];
    		}
    	}

    	
		var label = libzfs.getDeviceLabel(child['path']);
		if(label.name == "rpool"){
			return done(); 		
		}

		assert(false);
		done();
  })
})
