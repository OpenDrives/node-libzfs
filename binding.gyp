{
  "targets": [
    {
        "target_name": "node_libzfs",
        "conditions": [
	        ['OS=="linux"', {
	      		"include_dirs" : ["/usr/include/libspl/","/usr/include/libzfs/","<!(node -e \"require('nan')\")"],
	      		"libraries": ["/lib/libzfs.so.2"],
	      		"sources": [ "libzfs.cpp"]
	        }],
	        ['OS=="solaris"', {
				"include_dirs" : ["/usr/include/","<!(node -e \"require('nan')\")"],
	      		"libraries": ["/lib/libzfs.so"],
				"sources": [ "libzfs.cpp"],
	        }],
		],
		"include_dirs" : ["<!(node -e \"require('nan')\")"],
    }
  ],

}
