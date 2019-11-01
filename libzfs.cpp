/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2011 Pawel Jakub Dawidek. All rights reserved.
 * Copyright (c) 2011, 2015 by Delphix. All rights reserved.
 * Copyright (c) 2012, Joyent, Inc. All rights reserved.
 * Copyright (c) 2013 Steven Hartland. All rights reserved.
 * Copyright (c) 2014 Integros [integros.com]
 * Copyright 2016 Nexenta Systems, Inc.
 * Copyright 2017 OpenDrives, LLC
 */

#include <nan.h>
#include <iostream>
#include <libzfs.h>
#include <libzutil.h>
#include <unistd.h>

using namespace Nan;
using namespace v8;
using namespace std;

libzfs_handle_t *libhd;

#ifndef __linux

typedef enum {
    VDEV_NAME_PATH      = 1 << 0,
    VDEV_NAME_GUID      = 1 << 1,
    VDEV_NAME_FOLLOW_LINKS  = 1 << 2,
    VDEV_NAME_TYPE_ID   = 1 << 3,
} vdev_name_t;

/*
* solaris overload
*/
zpool_status_t zpool_get_status(zpool_handle_t* hndl, char** name,int* errata){
    return zpool_get_status(hndl,name);
}

void zfs_nicebytes(uint64_t num, char *buf, size_t buflen){
    zfs_nicenum(num, buf, buflen);
}

int zpool_read_label(int fd, nvlist_t ** nv, int* stub){
  return zpool_read_label(fd,nv);
}

#endif

typedef struct aux_cbdata {
    const char  *cb_type;
    uint64_t    cb_guid;
    zpool_handle_t  *cb_zhp;
} aux_cbdata_t;

typedef struct spare_cbdata {
    uint64_t    cb_guid;
    zpool_handle_t  *cb_zhp;
} spare_cbdata_t;

typedef struct vdev_cmd_data
{
    char **lines;   /* Array of lines of output, minus the column name */
    int lines_cnt;  /* Number of lines in the array */

    char **cols;    /* Array of column names */
    int cols_cnt;   /* Number of column names */


    char *path; /* vdev path */
    char *upath;    /* vdev underlying path */
    char *pool; /* Pool name */
    char *cmd;  /* backpointer to cmd */
    char *vdev_enc_sysfs_path;  /* enclosure sysfs path (if any) */
} vdev_cmd_data_t;

typedef struct vdev_cmd_data_list
{
    char *cmd;      /* Command to run */
    unsigned int count; /* Number of vdev_cmd_data items (vdevs) */

    /* vars to whitelist only certain vdevs, if requested */
    libzfs_handle_t *g_zfs;
    char **vdev_names;
    int vdev_names_count;
    int cb_name_flags;

    vdev_cmd_data_t *data;  /* Array of vdevs */

    /* List of unique column names and widths */
    char **uniq_cols;
    int uniq_cols_cnt;
    int *uniq_cols_width;

} vdev_cmd_data_list_t;

typedef struct status_cbdata {
    int     cb_count;
    int     cb_name_flags;
    int     cb_namewidth;
    boolean_t   cb_allpools;
    boolean_t   cb_verbose;
    boolean_t   cb_explain;
    boolean_t   cb_first;
    boolean_t   cb_dedup_stats;
    boolean_t   cb_print_status;
    vdev_cmd_data_list_t    *vcdl;
} status_cbdata_t;


/*
 * get configuration state as requested by status_callback.
 */
static v8::Local<v8::Object>
get_status_config(zpool_handle_t *zhp, status_cbdata_t *cb, const char *name,
    nvlist_t *nv, int depth, boolean_t isspare)
{

    // status config
    nvlist_t **child;
    uint_t c, children;
    // pool_scan_stat_t *ps = NULL;
    vdev_stat_t *vs;
    char rbuf[6], wbuf[6], cbuf[6];
    char *vname;
    int haslog = 0;
    uint64_t notpresent;
    // spare_cbdata_t spare_cb;
    char *state;
    char *path = NULL;
    char *physpath = NULL;
    v8::Local<v8::Object> config_obj = Nan::New<v8::Object>();
    char *type;


    /*
    * Get children
    */
    if (nvlist_lookup_nvlist_array(nv, ZPOOL_CONFIG_CHILDREN,
        &child, &children) != 0)
        children = 0;

    /*
    * Get Virtual Device Stats
    */
    verify(nvlist_lookup_uint64_array(nv, ZPOOL_CONFIG_VDEV_STATS,
        (uint64_t **)&vs, &c) == 0);

    /*
    * Get type
    */
    verify(nvlist_lookup_string(nv, ZPOOL_CONFIG_TYPE, &type) == 0);




    /*
    * Get State
    */
    state = (char*)zpool_state_to_name((vdev_state_t)vs->vs_state, (vdev_aux_t)vs->vs_aux);
    if (isspare) {
        /*
         * For hot spares, we use the terms 'INUSE' and 'AVAILABLE' for
         * online drives.
         */
        if (vs->vs_aux == VDEV_AUX_SPARED)
            state = (char*)"INUSE";
        else if (vs->vs_state == VDEV_STATE_HEALTHY)
            state = (char*)"AVAIL";
    }

    //  Physical Pathh

    if(nvlist_lookup_string(nv, ZPOOL_CONFIG_PHYS_PATH, &physpath) == 0 ){
            Nan::Set(config_obj, Nan::New<v8::String>("physpath").ToLocalChecked(),Nan::New<String>(physpath).ToLocalChecked());
    }
    /*
    * Get path
    */
    if(nvlist_lookup_string(nv, ZPOOL_CONFIG_PATH, &path) == 0){
        Nan::Set(config_obj, Nan::New<v8::String>("path").ToLocalChecked(),Nan::New<String>(path).ToLocalChecked());
    }

    // Vdev level errors
    zfs_nicenum(vs->vs_read_errors, rbuf, sizeof (rbuf));
    zfs_nicenum(vs->vs_write_errors, wbuf, sizeof (wbuf));
    zfs_nicenum(vs->vs_checksum_errors, cbuf, sizeof (cbuf));

    if (nvlist_lookup_uint64(nv, ZPOOL_CONFIG_NOT_PRESENT,
        &notpresent) == 0) { // When is a config not present
        verify(nvlist_lookup_string(nv, ZPOOL_CONFIG_PATH, &path) == 0);
        (void) printf("  was %s", path);
    } else if (vs->vs_aux != 0) {  // What is VS_AUX ?
        //vs->vs_aux
    }

    Nan::Set(config_obj, Nan::New<v8::String>("name").ToLocalChecked(),Nan::New<String>(name).ToLocalChecked());
    Nan::Set(config_obj, Nan::New<v8::String>("type").ToLocalChecked(),Nan::New<String>(type).ToLocalChecked());
    Nan::Set(config_obj, Nan::New<v8::String>("state").ToLocalChecked(),Nan::New<String>(state).ToLocalChecked());
    Nan::Set(config_obj, Nan::New<v8::String>("read_errors").ToLocalChecked(),Nan::New<String>(rbuf).ToLocalChecked());
    Nan::Set(config_obj, Nan::New<v8::String>("write_errors").ToLocalChecked(),Nan::New<String>(wbuf).ToLocalChecked());
    Nan::Set(config_obj, Nan::New<v8::String>("checksum_errors").ToLocalChecked(),Nan::New<String>(cbuf).ToLocalChecked());

    v8::Local<v8::Array> child_array =  Nan::New<v8::Array>(children);
    v8::Local<v8::Array> log_array;
    v8::Local<v8::Array> spares_array;
    v8::Local<v8::Array> cache_array;

    uint64_t islog = 0;

    for (c = 0; c < children; c++) {
        uint64_t ishole = 0, islog = 0;

        if (nvlist_lookup_uint64(child[c],
            ZPOOL_CONFIG_IS_HOLE, &ishole) == 0 && ishole)
            continue;

        if (nvlist_lookup_uint64(child[c],
            ZPOOL_CONFIG_IS_LOG, &islog) == 0 && islog) {
            haslog++;
            continue;
        }

        if (islog || ishole)
            continue;

        vname = zpool_vdev_name(libhd, zhp, child[c], (boolean_t)VDEV_NAME_TYPE_ID);

        v8::Local<v8::Object> conf = get_status_config(zhp, cb, vname, child[c], depth + 2,isspare);
        child_array->Set(c,conf);

        free(vname);
    }

    Nan::Set(config_obj, Nan::New<v8::String>("children").ToLocalChecked(),child_array);

    log_array =  Nan::New<v8::Array>(haslog);
    int i = 0;
    if (haslog>0) {
        for (c = 0; c < children; c++) {
            if (nvlist_lookup_uint64(child[c], ZPOOL_CONFIG_IS_LOG,
                &islog) != 0 || !islog)
                continue;
            vname = zpool_vdev_name(libhd, zhp, child[c], (boolean_t)VDEV_NAME_TYPE_ID);
            v8::Local<v8::Object> conf = get_status_config(zhp, cb,vname, child[c], depth + 2,isspare);
            log_array->Set(i,conf);
            i++;
            free(vname);
        }

    Nan::Set(config_obj, Nan::New<v8::String>("logs").ToLocalChecked(),log_array);
    }

    if (nvlist_lookup_nvlist_array(nv, ZPOOL_CONFIG_L2CACHE,
        &child, &children) == 0 && children > 0) {

        cache_array =  Nan::New<v8::Array>(children);

        for (c = 0; c < children; c++) {
            vname = zpool_vdev_name(libhd, zhp, child[c],
                (boolean_t)VDEV_NAME_TYPE_ID);
            v8::Local<v8::Object> conf = get_status_config(zhp, cb,vname, child[c], depth + 2,isspare);
            cache_array->Set(c,conf);
            free(vname);
        }

        Nan::Set(config_obj, Nan::New<v8::String>("cache").ToLocalChecked(),cache_array);
    }

    if (nvlist_lookup_nvlist_array(nv, ZPOOL_CONFIG_SPARES, &child,
        &children) == 0 && children > 0) {
        spares_array =  Nan::New<v8::Array>(children);

        for (c = 0; c < children; c++) {
            vname = zpool_vdev_name(libhd, zhp, child[c],
                (boolean_t)VDEV_NAME_TYPE_ID);

            v8::Local<v8::Object> conf = get_status_config(zhp, cb,vname, child[c], depth + 2,isspare);
            spares_array->Set(c,conf);
            free(vname);
        }

        Nan::Set(config_obj, Nan::New<v8::String>("spares").ToLocalChecked(),spares_array);

    }

    return config_obj;


}

NAN_METHOD(GetPoolStatus) {
   // get the param
    v8::String::Utf8Value param1(info[0]->ToString());

    // convert it to string
    std::string dataset = std::string(*param1);

    zpool_handle_t  *zhp;
    zpool_status_t status;
    vdev_stat_t *vs;
    nvlist_t *config, *nvroot;
    uint64_t pool_guid;
    uint_t c;
    int depth = 0;
    boolean_t isspare = (boolean_t)false;

    const char* health;

    char *msgid;

    status_cbdata_t * cb = { 0 };

    if((libhd = libzfs_init()) == NULL){
        //error
        printf("* Could not init libzfs");
    }else{
        if((zhp = zpool_open(libhd,dataset.c_str())) == NULL){
            libzfs_fini(libhd);
            // printf("* Could not open the dataset [%s]\n",dataset.c_str());

        }else{
            config = zpool_get_config(zhp,NULL);

            (void) nvlist_lookup_uint64(config,ZPOOL_CONFIG_GUID,&pool_guid);
            (void) nvlist_lookup_nvlist(config,ZPOOL_CONFIG_VDEV_TREE,&nvroot);
            verify(nvlist_lookup_uint64_array(nvroot,ZPOOL_CONFIG_VDEV_STATS,(uint64_t **)&vs,&c) == 0 );
            health = zpool_state_to_name((vdev_state_t)vs->vs_state,(vdev_aux_t)vs->vs_aux);
            status = zpool_get_status(zhp,&msgid,NULL);
            v8::Local<v8::Object> pool_obj = Nan::New<v8::Object>();
            v8::Local<v8::Object> scan_obj = Nan::New<v8::Object>();
            v8::Local<v8::Object> conf_obj = Nan::New<v8::Object>();

            if(config != NULL){

                pool_scan_stat_t *ps = NULL;
                (void) nvlist_lookup_uint64_array(nvroot,ZPOOL_CONFIG_SCAN_STATS, (uint64_t **)&ps, &c);


                time_t start, end;
                uint64_t elapsed, mins_left;
                uint64_t hours_left = 0;
                uint64_t pass_exam, examined,to_examine, total;
                uint_t rate;
                double fraction_done;
                char examined_buf[7],to_examine_buf[7], total_buf[7];


                /* If there's never been a scan, there's not much to say. */
                if (ps == NULL || ps->pss_func == POOL_SCAN_NONE ||
                    ps->pss_func >= POOL_SCAN_FUNCS) {

                }else{

                    start = ps->pss_start_time;
                    end = ps->pss_end_time;

                    examined = ps->pss_examined ? ps->pss_examined : 1;
                    total = ps->pss_to_examine;
                    to_examine = ps->pss_to_examine;
                    fraction_done = (double)examined / total;

                    /* elapsed time for this pass */
                    elapsed = time(NULL) - ps->pss_pass_start;
                    // elapsed -= ps->pss_pass_scrub_spent_paused;
                    elapsed = elapsed ? elapsed : 1;
                    pass_exam = ps->pss_pass_exam ? ps->pss_pass_exam : 1;
                    rate = pass_exam / elapsed;
                    rate = rate ? rate : 1;
                    mins_left = ((total - examined) / rate) / 60;
                    hours_left = mins_left / 60;


                    zfs_nicebytes(examined, examined_buf, sizeof (examined_buf));
                    zfs_nicebytes(to_examine, to_examine_buf, sizeof (to_examine_buf));
                    zfs_nicebytes(total, total_buf, sizeof (total_buf));
                    if(ps->pss_state == 1){
                        Nan::Set(scan_obj, Nan::New<v8::String>("hours_left").ToLocalChecked(),Nan::New<Number>(static_cast<unsigned long> (hours_left)));
                        Nan::Set(scan_obj, Nan::New<v8::String>("mins_left").ToLocalChecked(),Nan::New<Number>(static_cast<unsigned long> (mins_left)));
                        Nan::Set(scan_obj, Nan::New<v8::String>("rate").ToLocalChecked(),Nan::New<Number>(static_cast<unsigned int> (rate)));

                    }

                    Nan::Set(scan_obj, Nan::New<v8::String>("state").ToLocalChecked(),Nan::New<Number>(ps->pss_state));
                    Nan::Set(scan_obj, Nan::New<v8::String>("start").ToLocalChecked(),Nan::New<Number>(static_cast<long int> (start)));
                    Nan::Set(scan_obj, Nan::New<v8::String>("end").ToLocalChecked(),Nan::New<Number>(static_cast<long int> (end)));
                    Nan::Set(scan_obj, Nan::New<v8::String>("function").ToLocalChecked(),Nan::New<Number>(ps->pss_func));
                    Nan::Set(scan_obj, Nan::New<v8::String>("examined").ToLocalChecked(),Nan::New<Number>( examined ));
                    Nan::Set(scan_obj, Nan::New<v8::String>("examined_hr").ToLocalChecked(),Nan::New<String>(examined_buf).ToLocalChecked());
                    Nan::Set(scan_obj, Nan::New<v8::String>("to_examine").ToLocalChecked(),Nan::New<Number>( to_examine));
                    Nan::Set(scan_obj, Nan::New<v8::String>("to_examine_hr").ToLocalChecked(),Nan::New<String>(to_examine_buf).ToLocalChecked());
                    Nan::Set(scan_obj, Nan::New<v8::String>("total").ToLocalChecked(),Nan::New<Number>(total));
                    Nan::Set(scan_obj, Nan::New<v8::String>("total_hr").ToLocalChecked(),Nan::New<String>(total_buf).ToLocalChecked());
                    Nan::Set(scan_obj, Nan::New<v8::String>("fraction_done").ToLocalChecked(),Nan::New<Number>(fraction_done));

                }
                conf_obj = get_status_config(zhp, cb, dataset.c_str(), nvroot, depth + 2,isspare);

            }


            Nan::Set(pool_obj, Nan::New<v8::String>("name").ToLocalChecked(),Nan::New<String>(dataset).ToLocalChecked());
            Nan::Set(pool_obj, Nan::New<v8::String>("guid").ToLocalChecked(),Nan::New<String>(std::to_string(pool_guid)).ToLocalChecked());
            Nan::Set(pool_obj, Nan::New<v8::String>("state").ToLocalChecked(),Nan::New<String>(health).ToLocalChecked());
            Nan::Set(pool_obj, Nan::New<v8::String>("status").ToLocalChecked(),Nan::New<Number>((int)status));
            Nan::Set(pool_obj, Nan::New<v8::String>("scan").ToLocalChecked(),scan_obj);
            Nan::Set(pool_obj, Nan::New<v8::String>("config").ToLocalChecked(),conf_obj);


            info.GetReturnValue().Set(pool_obj);

            zpool_close(zhp);
            libzfs_fini(libhd);
        }
    }

}

NAN_METHOD(GetEnumValue) {
    v8::String::Utf8Value param1(info[0]->ToString());
    // convert it to string
    std::string enumname = std::string(*param1);
    info.GetReturnValue().Set(Nan::New<v8::String>("UNKNOWN!").ToLocalChecked());

    int value  = info[1]->Uint32Value();

    if(enumname== "status"){
        switch(value){
            case ZPOOL_STATUS_CORRUPT_CACHE:
                info.GetReturnValue().Set(Nan::New<v8::String>("CORRUPT_CACHE").ToLocalChecked());
                break;
            case ZPOOL_STATUS_MISSING_DEV_R:
                info.GetReturnValue().Set(Nan::New<v8::String>("MISSING_DEV_R").ToLocalChecked());
                break;
            case ZPOOL_STATUS_MISSING_DEV_NR:
                info.GetReturnValue().Set(Nan::New<v8::String>("MISSING_DEV_NR").ToLocalChecked());
                break;
            case ZPOOL_STATUS_CORRUPT_LABEL_R:
                info.GetReturnValue().Set(Nan::New<v8::String>("CORRUPT_LABEL_R").ToLocalChecked());
                break;
            case ZPOOL_STATUS_CORRUPT_LABEL_NR:
                info.GetReturnValue().Set(Nan::New<v8::String>("CORRUPT_LABEL_NR").ToLocalChecked());
                break;
            case ZPOOL_STATUS_BAD_GUID_SUM:
                info.GetReturnValue().Set(Nan::New<v8::String>("BAD_GUID_SUM").ToLocalChecked());
                break;
            case ZPOOL_STATUS_CORRUPT_POOL:
                info.GetReturnValue().Set(Nan::New<v8::String>("CORRUPT_POOL").ToLocalChecked());
                break;
            case ZPOOL_STATUS_CORRUPT_DATA:
                info.GetReturnValue().Set(Nan::New<v8::String>("CORRUPT_DATA").ToLocalChecked());
                break;
            case ZPOOL_STATUS_FAILING_DEV:
                info.GetReturnValue().Set(Nan::New<v8::String>("FAILING_DEV").ToLocalChecked());
                break;
            case ZPOOL_STATUS_VERSION_NEWER:
                info.GetReturnValue().Set(Nan::New<v8::String>("VERSION_NEWER").ToLocalChecked());
                break;
            case ZPOOL_STATUS_HOSTID_MISMATCH:
                info.GetReturnValue().Set(Nan::New<v8::String>("HOSTID_MISMATCH").ToLocalChecked());
                break;
            case ZPOOL_STATUS_IO_FAILURE_WAIT:
                info.GetReturnValue().Set(Nan::New<v8::String>("IO_FAILURE_WAIT").ToLocalChecked());
                break;
            case ZPOOL_STATUS_IO_FAILURE_CONTINUE:
                info.GetReturnValue().Set(Nan::New<v8::String>("IO_FAILURE_CONTINUE").ToLocalChecked());
                break;
            case ZPOOL_STATUS_BAD_LOG:
                info.GetReturnValue().Set(Nan::New<v8::String>("BAD_LOG").ToLocalChecked());
                break;
            case ZPOOL_STATUS_UNSUP_FEAT_READ:
                info.GetReturnValue().Set(Nan::New<v8::String>("UNSUP_FEAT_READ").ToLocalChecked());
                break;
            case ZPOOL_STATUS_UNSUP_FEAT_WRITE:
                info.GetReturnValue().Set(Nan::New<v8::String>("UNSUP_FEAT_WRITE").ToLocalChecked());
                break;
            case ZPOOL_STATUS_FAULTED_DEV_R:
                info.GetReturnValue().Set(Nan::New<v8::String>("FAULTED_DEV_R").ToLocalChecked());
                break;
            case ZPOOL_STATUS_FAULTED_DEV_NR:
                info.GetReturnValue().Set(Nan::New<v8::String>("FAULTED_DEV_NR").ToLocalChecked());
                break;
            case ZPOOL_STATUS_VERSION_OLDER:
                info.GetReturnValue().Set(Nan::New<v8::String>("VERSION_OLDER").ToLocalChecked());
                break;

            case ZPOOL_STATUS_FEAT_DISABLED:
                info.GetReturnValue().Set(Nan::New<v8::String>("FEAT_DISABLED").ToLocalChecked());
                break;
            case ZPOOL_STATUS_RESILVERING:
                info.GetReturnValue().Set(Nan::New<v8::String>("RESILVERING").ToLocalChecked());
                break;
            case ZPOOL_STATUS_OFFLINE_DEV:
                info.GetReturnValue().Set(Nan::New<v8::String>("OFFLINE_DEV").ToLocalChecked());
                break;

            case ZPOOL_STATUS_REMOVED_DEV:
                info.GetReturnValue().Set(Nan::New<v8::String>("REMOVED_DEV").ToLocalChecked());
                break;

            case ZPOOL_STATUS_OK:
                info.GetReturnValue().Set(Nan::New<v8::String>("OK").ToLocalChecked());
                break;
        }
    }else if(enumname== "pool_state"){
        switch(value){
            case POOL_STATE_ACTIVE:
                info.GetReturnValue().Set(Nan::New<v8::String>("POOL_ACTIVE").ToLocalChecked());
                break;
            case POOL_STATE_EXPORTED:
                info.GetReturnValue().Set(Nan::New<v8::String>("EXPORTED").ToLocalChecked());
                break;
            case POOL_STATE_DESTROYED:
                info.GetReturnValue().Set(Nan::New<v8::String>("DESTROYED").ToLocalChecked());
                break;
            case POOL_STATE_SPARE:
                info.GetReturnValue().Set(Nan::New<v8::String>("SPARE").ToLocalChecked());
                break;
            case POOL_STATE_L2CACHE:
                info.GetReturnValue().Set(Nan::New<v8::String>("L2CACHE").ToLocalChecked());
                break;
            case POOL_STATE_UNINITIALIZED:
                info.GetReturnValue().Set(Nan::New<v8::String>("UNINITIALIZED").ToLocalChecked());
                break;
            case POOL_STATE_UNAVAIL:
                info.GetReturnValue().Set(Nan::New<v8::String>("UNAVAIL").ToLocalChecked());
                break;
            case POOL_STATE_POTENTIALLY_ACTIVE:
                info.GetReturnValue().Set(Nan::New<v8::String>("POTENTIALLY_ACTIVE").ToLocalChecked());
                break;
        }
    }else if(enumname== "scan_state"){
        switch(value){
            case DSS_NONE:
                info.GetReturnValue().Set(Nan::New<v8::String>("NONE").ToLocalChecked());
                break;
            case DSS_SCANNING:
                info.GetReturnValue().Set(Nan::New<v8::String>("SCANNING").ToLocalChecked());
                break;
            case DSS_FINISHED:
                info.GetReturnValue().Set(Nan::New<v8::String>("FINISHED").ToLocalChecked());
                break;
            case DSS_CANCELED:
                info.GetReturnValue().Set(Nan::New<v8::String>("CANCELED").ToLocalChecked());
                break;
            case DSS_NUM_STATES:
                info.GetReturnValue().Set(Nan::New<v8::String>("NUM_STATES").ToLocalChecked());
        }
    }else if(enumname== "scan_function"){
        switch(value){
            case POOL_SCAN_NONE:
                info.GetReturnValue().Set(Nan::New<v8::String>("NONE").ToLocalChecked());
                break;
            case POOL_SCAN_SCRUB:
                info.GetReturnValue().Set(Nan::New<v8::String>("SCRUB").ToLocalChecked());
                break;
            case POOL_SCAN_RESILVER:
                info.GetReturnValue().Set(Nan::New<v8::String>("RESILVER").ToLocalChecked());
                break;
            case POOL_SCAN_FUNCS:
                info.GetReturnValue().Set(Nan::New<v8::String>("FUNCS").ToLocalChecked());
                break;
        }
    }else{
        info.GetReturnValue().Set(false);
    }


}


//https://github.com/illumos/illumos-gate/blob/ed992b0aac4e5b70dc1273b1d055c0d471fbb4b1/usr/src/lib/libzfs/common/libzfs_import.c#L1473
static int
find_aux(zpool_handle_t *zhp, void *data)
{
    aux_cbdata_t *cbp = (aux_cbdata_t*)data;
    nvlist_t **list;
    uint_t i, count;
    uint64_t guid;
    nvlist_t *nvroot,*config;
    config = zpool_get_config(zhp,NULL);

    verify(nvlist_lookup_nvlist(config, ZPOOL_CONFIG_VDEV_TREE,&nvroot) == 0);

    if (nvlist_lookup_nvlist_array(nvroot, cbp->cb_type,
        &list, &count) == 0) {
        for (i = 0; i < count; i++) {
            verify(nvlist_lookup_uint64(list[i],
                ZPOOL_CONFIG_GUID, &guid) == 0);
            if (guid == cbp->cb_guid) {
                cbp->cb_zhp = zhp;
                return (1);
            }
        }
    }
    zpool_close(zhp);
    return (0);
}

NAN_METHOD(ReadLabel) {
   // get the param
    v8::String::Utf8Value param1(info[0]->ToString());

    // convert it to string
    std::string dataset = std::string(*param1);

    nvlist_t* list;

    int fd;
    v8::Local<v8::Object> item;
    aux_cbdata_t cb = {
        0,
        0,
        0,
    };

    fd = open(dataset.c_str(),O_RDONLY);
        if (zpool_read_label(fd, &list,NULL) != 0 || list == NULL) {
            // printf("* Could not read label on [%s]\n",dataset.c_str());
        }else{
            item = Nan::New<v8::Object>();

            uint64_t guid =0;
            if(nvlist_lookup_uint64(list, ZPOOL_CONFIG_GUID, &guid) == 0){
                Nan::Set(item, Nan::New<v8::String>("conf_guid").ToLocalChecked(),Nan::New<v8::String>(std::to_string(guid)).ToLocalChecked());
            }


            uint64_t pool_guid;
            if(nvlist_lookup_uint64(list, ZPOOL_CONFIG_POOL_GUID, &pool_guid) == 0){
                Nan::Set(item, Nan::New<v8::String>("pool_guid").ToLocalChecked(),Nan::New<v8::String>(std::to_string(pool_guid)).ToLocalChecked());
            }

            uint64_t state;
            if(nvlist_lookup_uint64(list, ZPOOL_CONFIG_POOL_STATE,&state) == 0){
                Nan::Set(item, Nan::New<v8::String>("state").ToLocalChecked(),Nan::New<Number>((int)state));
            }

            char *name = NULL;

            if (state == POOL_STATE_L2CACHE || state == POOL_STATE_SPARE){
                if((libhd = libzfs_init()) == NULL){
                    //error
                    printf("* Could not init libzfs");
                }

                /*
                 * Check if any pool is currently using this l2cache device or as a spare ( returns first pool found using it as a spare)
                 */
                cb.cb_zhp = NULL;
                cb.cb_guid = guid;
                switch(state){
                    case POOL_STATE_L2CACHE:
                        cb.cb_type = ZPOOL_CONFIG_L2CACHE;
                        break;
                    case POOL_STATE_SPARE:
                        cb.cb_type = ZPOOL_CONFIG_SPARES;
                        break;
                }

                if (zpool_iter(libhd, find_aux, &cb) == 1) {
                    name = (char *)zpool_get_name(cb.cb_zhp);
                }
                libzfs_fini(libhd);
            }else{
                nvlist_lookup_string(list, ZPOOL_CONFIG_POOL_NAME, &name);
            }

            if(name != NULL){
                Nan::Set(item, Nan::New<v8::String>("name").ToLocalChecked(),Nan::New<v8::String>(name).ToLocalChecked());
            }

        }

        info.GetReturnValue().Set(item);


    (void) close(fd);


}

NAN_MODULE_INIT(Init) {

   Nan::Set(target, New<String>("ReadLabel").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(ReadLabel)).ToLocalChecked());
   Nan::Set(target, New<String>("GetPoolStatus").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(GetPoolStatus)).ToLocalChecked());
   Nan::Set(target, New<String>("GetEnumValue").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(GetEnumValue)).ToLocalChecked());
}

NODE_MODULE(node_libzfs, Init)
