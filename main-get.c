/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <margo.h>

#include "sdskv-client.h"
#include "sdskv-common.h"

#include "pmpi-common.h"

int main(int argc, char *argv[])
{
    char cli_addr_prefix[64] = {0};
    char *sdskv_svr_addr_str;
    char *db_name;
    margo_instance_id mid;
    hg_addr_t svr_addr;
    uint8_t mplex_id;
    uint32_t num_keys;
    sdskv_client_t kvcl;
    sdskv_provider_handle_t kvph;
    hg_return_t hret;
    int ret;
    

    if(argc != 5)
    {
        fprintf(stderr, "Usage: %s <sdskv_server_addr> <mplex_id> <db_name> <num_keys>\n", argv[0]);
        fprintf(stderr, "  Example: %s tcp://localhost:1234 1 foo 1000\n", argv[0]);
        return(-1);
    }
    sdskv_svr_addr_str = argv[1];
    mplex_id          = atoi(argv[2]);
    db_name           = argv[3];
    num_keys          = atoi(argv[4]);

    /* initialize Margo using the transport portion of the server
     * address (i.e., the part before the first : character if present)
     */
    for(unsigned i=0; (i<63 && sdskv_svr_addr_str[i] != '\0' && sdskv_svr_addr_str[i] != ':'); i++)
        cli_addr_prefix[i] = sdskv_svr_addr_str[i];

    /* start margo */
    mid = margo_init(cli_addr_prefix, MARGO_SERVER_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL)
    {
        fprintf(stderr, "Error: margo_init()\n");
        return(-1);
    }

    ret = sdskv_client_init(mid, &kvcl);
    if(ret != 0)
    {
        fprintf(stderr, "Error: sdskv_client_init()\n");
        margo_finalize(mid);
        return -1;
    }

    /* look up the SDSKV server address */
    hret = margo_addr_lookup(mid, sdskv_svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS)
    {
        fprintf(stderr, "Error: margo_addr_lookup()\n");
        sdskv_client_finalize(kvcl);
        margo_finalize(mid);
        return(-1);
    }

    /* create a SDSKV provider handle */
    ret = sdskv_provider_handle_create(kvcl, svr_addr, mplex_id, &kvph);
    if(ret != 0)
    {
        fprintf(stderr, "Error: sdskv_provider_handle_create()\n");
        margo_addr_free(mid, svr_addr);
        sdskv_client_finalize(kvcl);
        margo_finalize(mid);
        return(-1);
    }

    /* open the database */
    printf("\n\n\n");
    printf("main-get: Getting key-value pairs from database\n");
    sdskv_database_id_t db_id;
    printf("main-get: kvph is %u\n", (unsigned) kvph);
    printf("main-get: db_name is %s\n", db_name);
    ret = sdskv_open(kvph, db_name, &db_id);
    if(ret == 0) {
        printf("main-get: Successfuly opened database %s, id is %ld\n", db_name, db_id);
    } else {
        fprintf(stderr, "Error: could not open database %s\n", db_name);
        sdskv_provider_handle_release(kvph);
        margo_addr_free(mid, svr_addr);
        sdskv_client_finalize(kvcl);
        margo_finalize(mid);
        return(-1);
    }

    /* **** get keys **** */

    char* key1 = "test";
    unsigned ksize = strlen(key1);


    unsigned long * value = malloc(sizeof(unsigned long));
    unsigned vsize = sizeof(unsigned long);
    unsigned long vsizeget = sizeof(unsigned long);

    //int value1 = 1;
    //int value2 = 2;
    //ret = sdskv_put(kvph, db_id,
    //                (const void*) key1, ksize,
    //                (const void*) &value1, vsize);

    //
    //ret = sdskv_get(kvph, db_id,
    //		    (const void *) key1, ksize,
    //		    (void *) value, &vsizeget);

    //printf("main-get: kv-pair: %s => %d\n", key1, *(int*) value);


    //ret = sdskv_put(kvph, db_id,
    //                (const void*) key1, ksize,
    //                (const void*) &value2, vsize);

    //ret = sdskv_get(kvph, db_id,
    //		    (const void *) key1, ksize,
    //		    (void *) value, &vsizeget);

    //printf("main-get: kv-pair: %s => %d\n", key1, *(int*) value);
   

    printf("\n\n\n\n======== getting kv-pairs from DB ========\n\n\n\n");
    unsigned i;
    char* filename = "keys.txt";
    char** keylist; 
    unsigned numkeys = 0;
    get_keys_from_file(filename, &keylist, &numkeys);
    printf("main-get: numkeys = %u\n", numkeys);
    for (i = 0; i < numkeys; i++) {
	printf("main-get: key[%u] is '%s'\n", i, keylist[i]);
    }
    printf("\n\n\n\n");

    char* key;
    int exists = 0;
    for (i=0; i < numkeys; i++) {
        key = keylist[i];
	
	printf("main-get: looking for key '%s'...", key);
    	ksize = strlen(key);
    	sdskv_exists(kvph, db_id,
    		     (const void*) key, ksize,
    		     &exists);

    	if (exists) {
	    printf("main-get: Exists!\n");
    	    ret = sdskv_get(kvph, db_id,
			    (const void *) key, ksize,
			    (void *) value, &vsizeget);
    	    if(ret != 0) {
    		fprintf(stderr, "Error: sdskv_get() failed (key was %s)\n", key);
    		if (SDSKV_ERROR_IS_HG(ret)) {
    		    printf("main-get: ERROR IS HG\n");
    		}
    		if (SDSKV_ERROR_IS_ABT(ret)) {
    		    printf("main-get: ERROR IS ABT\n");
    		}
    		sdskv_shutdown_service(kvcl, svr_addr);
    		sdskv_provider_handle_release(kvph);
    		margo_addr_free(mid, svr_addr);
    		sdskv_client_finalize(kvcl);
    		margo_finalize(mid);
    		return -1;
    	    }
    	    printf("main-get: Key = %s, value = %lu, vsize = %u\n", key,  *value, vsize);
    	    printf("\n");
    	} else {
    	    printf("Does Not Exist!\n\n");
    	}
    }
    free(value);
    free_keylist(&keylist, numkeys);
    printf("\n\n\n\n========= Done getting kv-pairs =========\n\n\n\n");

    printf("main-get: Shutting down server and cleaning up...");
    /* shutdown the server */
    ret = sdskv_shutdown_service(kvcl, svr_addr);

    /**** cleanup ****/
    sdskv_provider_handle_release(kvph);
    margo_addr_free(mid, svr_addr);
    sdskv_client_finalize(kvcl);
    margo_finalize(mid);
    printf("done.\n");
    return(ret);
}

