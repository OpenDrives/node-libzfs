var assert = require('assert');
var libzfs = require('../');

describe('get Pool', function(){

  it('can load libzfs', function(done){
    assert(libzfs != null);
    done();
  });



  it('get rpool', function(done){
    var rpool = libzfs.getPoolStatus('rpool');
    assert(rpool)
    done();
  });

  it('fail to get missing pool', function(done){
    var foo = libzfs.getPoolStatus('foobar');
    assert(!foo);
    done();
  });

})
