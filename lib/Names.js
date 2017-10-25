module.exports = {

	// https://github.com/illumos/illumos-gate/blob/f7877f5d39900cfd8b20dd673e5ccc1ef7cc7447/usr/src/grub/grub-0.97/stage2/zfs-include/zfs.h#L106
	// change to getLabelStates
	getLabelStateNames:function(){ // not 100% happy with this
		return [
			"ACTIVE",		        /* In active use		*/
			"EXPORTED",		        /* Explicitly exported		*/
			"DESTROYED",	     	/* Explicitly destroyed		*/
			"SPARE",	        	/* Reserved for hot spare use	*/
			"L2CACHE",	        	/* Level 2 ARC device		*/
			"UNINITIALIZED",	    /* Internal spa_t state		*/
			"UNAVAIL",		        /* Internal libzfs state	*/
			"POTENTIALLY_ACTIVE"	/* Internal libzfs state	*/
		];
	},
	// https://github.com/illumos/illumos-gate/blob/1702cce751c5cb7ead878d0205a6c90b027e3de8/usr/src/lib/libzfs/common/libzfs.h#L297
	getPoolStatusNames:function(){ // not 100% happy with this
		return [
			/*
		     * The following correspond to faults as defined in the (fault.fs.zfs.*)
		     * event namespace.  Each is associated with a corresponding message ID.
		     */
		    "CORRUPT_CACHE",    /* corrupt /kernel/drv/zpool.cache */
		    "MISSING_DEV_R",    /* missing device with replicas */
		    "MISSING_DEV_NR",    /* missing device with no replicas */
		    "CORRUPT_LABEL_R",    /* bad device label with replicas */
		    "CORRUPT_LABEL_NR",    /* bad device label with no replicas */
		    "BAD_GUID_SUM",    /* sum of device guids didn't match */
		    "CORRUPT_POOL",    /* pool metadata is corrupted */
		    "CORRUPT_DATA",    /* data errors in user (meta)data */
		    "FAILING_DEV",    /* device experiencing errors */
		    "VERSION_NEWER",    /* newer on-disk version */
		    "HOSTID_MISMATCH",    /* last accessed by another system */
		    "IO_FAILURE_WAIT",    /* failed I/O", failmode 'wait' */
		    "IO_FAILURE_CONTINUE", /* failed I/O", failmode 'continue' */
		    "BAD_LOG",        /* cannot read log chain(s) */
			"ERRATA",		/* informational errata available */

			/*
			 * If the pool has unsupported features but can still be opened in
			 * read-only mode", its status is UNSUP_FEAT_WRITE. If the
			 * pool has unsu	pported features but cannot be opened at all", its
			 * statu		s is UNSUP_FEAT_READ.
			 */
		    "UNSUP_FEAT_READ",    /* unsupported features for read */
		    "UNSUP_FEAT_WRITE",    /* unsupported features for write */

			/*
			 * These faults have no corresponding message ID.  At the time we are
			 * checking the status", the original reason for the FMA fault (I/O or
			 * checksum errors) has been lost.
			 */
		    "FAULTED_DEV_R",    /* faulted device with replicas */
		    "FAULTED_DEV_NR",    /* faulted device with no replicas */

			/*
			 * The following are not faults per se", but still an error possibly
			 * requiring administrative attention.  There is no corresponding
			 * message ID.
			 */
		    "VERSION_OLDER",    /* older legacy on-disk version */
		    "FEAT_DISABLED",    /* supported features are disabled */
		    "RESILVERING",    /* device being resilvered */
		    "OFFLINE_DEV",    /* device offline */
		    "REMOVED_DEV" ,    /* removed device */

		    /*
		     * Finally", the following indicates a healthy pool.
		     */
		    "OK"
	    ]
	},








	
	//https://github.com/illumos/illumos-gate/blob/1702cce751c5cb7ead878d0205a6c90b027e3de8/usr/src/uts/common/sys/fs/zfs.h#L744
	getScanStateNames:function(){ // not 100% happy with this
		return [
			"NONE",
			"SCANNING",
			"FINISHED",
			"CANCELED",
			"NUM_STATES"
		]
	},

	//https://github.com/illumos/illumos-gate/blob/1702cce751c5cb7ead878d0205a6c90b027e3de8/usr/src/uts/common/sys/fs/zfs.h#L690
	getPoolScanFunctionNames:function(){
		return [
			"NONE",
			"SCRUB",
			"RESILVER",
			"FUNCS"
		]
	}

}