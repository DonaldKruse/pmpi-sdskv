#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ssg.h>
#include <ssg-mpi.h>


#include "pmpi-lib.h"
#include "pmpi-common.h"

#include <margo.h>
#include "../include/sdskv-client.h"
#include "../include/sdskv-server.h"
#include "../include/sdskv-common.h"


sdskv_provider_handle_t kvph;
sdskv_database_id_t db_id;
sdskv_client_t kvcl;
hg_addr_t svr_addr;
margo_instance_id mid;

void get_keys_from_file(const char* filename, char*** keylist, unsigned* numkeys)
{
    int i;
    int buffsize = 64;
    char buffer[buffsize];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
	(*keylist) = NULL;
	return;
    }
    
    while( (fgets(buffer, buffsize, fp)) != NULL) {
	(*numkeys)++;
    }
    (*numkeys)--; // we overcounted by 1

    (*keylist) = (char**) malloc((*numkeys)*sizeof(char*));
    rewind(fp);
    for (i = 0; i < (*numkeys); i++) {
	(*keylist)[i] = (char*) malloc(buffsize*sizeof(char));
	fscanf(fp, "%s[^\n]", (*keylist)[i]);
    }
    fclose(fp);
}

void free_keylist(char*** keylist, unsigned numkeys) {
    int i;
    for (i = 0; i < numkeys; i++) {
	free((*keylist)[i]);
	(*keylist)[i] = NULL;
    }
    (*keylist) = NULL;
}

void update_count_sdskv_put(unsigned *local_count, 
			  unsigned *total_count,
			  const unsigned threshold,
			    const char* key,
			    const hg_size_t dsize)
{
    (*local_count)++;
    if ((*local_count) >= threshold) {
	unsigned ksize = strlen(key);
	*total_count += *local_count;
	sdskv_put_check_err( (const void*) key, ksize,
			     (const void*) total_count, dsize);
	*local_count = 0;
    }
}



void get_parameters()
{
    const char* filename = "pmpi-params.txt";
    FILE* fi = NULL;
    int ret;
    fi = fopen(filename, "r");
    if (!fi) {
	printf("Could not open %s. Quitting...\n", filename);
	exit(1);
    }
    if ((ret = fscanf(fi, "%u", &global_threshold_recv))  == EOF) {
	printf("EOF found before scanning value for `global_threshold_recv`.");
	printf(" Quitting...\n");
	exit(1);
    }
    if ((ret = fscanf(fi, "%u", &global_threshold_isend))  == EOF) {
	printf("EOF found before scanning value for `global_threshold_isend`.");
	printf(" Quitting...\n");
	exit(1);
    }
    if ((ret = fscanf(fi, "%u", &global_threshold_send))  == EOF) {
	printf("EOF found before scanning value for `global_threshold_send`.");
	printf(" Quitting...\n");
	exit(1);
    }
    if ((ret = fscanf(fi, "%u", &global_threshold_reduce))  == EOF) {
	printf("EOF found before scanning value for `global_threshold_reduce`.");
	printf(" Quitting...\n");
	exit(1);
    }

    fclose(fi);
}

static void my_membership_update_cb(void* uargs,
                                    ssg_member_id_t member_id,
                                    ssg_member_update_type_t update_type)
{
    switch(update_type) {
    case SSG_MEMBER_JOINED:
        printf("Member %ld joined\n", member_id);
        break;
    case SSG_MEMBER_LEFT:
        printf("Member %ld left\n", member_id);
        break;
    case SSG_MEMBER_DIED:
        printf("Member %ld died\n", member_id);
        break;
    }
}

/**
 * Puts a key-value pair in the sdskv database and checks
 * for errors. If any errors happen during the sdskv_put,
 * this function shuts down and frees the server, and returns
 * -1, otherwise returns the return value from sdskv_put.
 */
int sdskv_put_check_err(const void *key, hg_size_t ksize,
                        const void *value, hg_size_t vsize)
{
    int ret;
    int rank;
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //printf("Rank %d, putting kv: %s => %d\n", rank, (char*) key, *(int*) value);
    ret = sdskv_put(kvph, db_id,
                    (const void*) key, ksize,
                    (const void*) value, vsize);
    if(ret != 0) {
        printf("Error: Rank %d sdskv_put() failed for kv pair (%s, %d). ret = %X\n",
                rank, (char*) key, *(int*) value, ret);
        printf("Rank %d will not do anything about it...\n", rank);
    } 
    return ret;
}

int sdskv_shutdown_service_cleanup() {
    int ret = 0;
    int rank;
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Rank %d is shutting down sdskv service...\n", rank);
    /* shutdown the server */
    ret = sdskv_shutdown_service(kvcl, svr_addr);

    /**** cleanup ****/
    sdskv_provider_handle_release(kvph);
    margo_addr_free(mid, svr_addr);
    sdskv_client_finalize(kvcl);
    margo_finalize(mid);

    return ret;
}


int init_margo_open_db_check_error(int* argc, char*** argv) {
    int ret;
    int ssg_ret;

    char cli_addr_prefix[64] = {0};
    char *sdskv_svr_addr_str;
    char *db_name;
    hg_addr_t svr_addr;
    uint8_t mplex_id;
    uint32_t num_keys;
    hg_return_t hret;
    int rank;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
    sdskv_svr_addr_str = (*argv)[1];
    printf("the server address is %s\n", sdskv_svr_addr_str);
    mplex_id          = atoi((*argv)[2]);
    db_name           = (*argv)[3];
    num_keys          = atoi((*argv)[4]);


    /* initialize Margo using the transport portion of the server
     * address (i.e., the part before the first : character if present)
     */
    for(unsigned i=0; (i<63 && sdskv_svr_addr_str[i] != '\0' && sdskv_svr_addr_str[i] != ':'); i++)
        cli_addr_prefix[i] = sdskv_svr_addr_str[i];

    /* Create the MPI group */
    //ssg_ret = ssg_init();
    //assert(ssg_ret == SSG_SUCCESS);

    /* start margo */
    mid = margo_init(cli_addr_prefix, MARGO_SERVER_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL)
    {
        printf("Rank %d, Error: margo_init()\n", rank);
        return(-1);
    } else {
        printf("Rank %d, margo_init() success\n", rank);
    }
    PMPI_Barrier(MPI_COMM_WORLD);




    //ssg_group_config_t config = {
    //    .swim_period_length_ms = 1000,
    //    .swim_suspect_timeout_periods = 5,
    //    .swim_subgroup_member_count = -1,
    //    .ssg_credential = -1
    //};
    // 
    //ssg_group_id_t gid = ssg_group_create_mpi(
    //    mid, "mygroup", MPI_COMM_WORLD,
    //    &config, my_membership_update_cb, NULL);
    //PMPI_Barrier(MPI_COMM_WORLD);




    


    ret = sdskv_client_init(mid, &kvcl);
    if(ret != 0)
    {
        fprintf(stderr, "Rank %d, Error: sdskv_client_init()\n", rank);
        margo_finalize(mid);
        return -1;
    } else {
        printf("Rank %d: sdskv_client_init() success\n", rank);
    }
    PMPI_Barrier(MPI_COMM_WORLD);

    /* look up the SDSKV server address */
    hret = margo_addr_lookup(mid, sdskv_svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS)
    {
        printf("Rank %d, Error: margo_addr_lookup()\n", rank);
        sdskv_client_finalize(kvcl);
        margo_finalize(mid);
        return(-1);
    } else {
        printf("Rank %d: margo_addr_lookup() success\n", rank);
    }
    PMPI_Barrier(MPI_COMM_WORLD);

    /* create a SDSKV provider handle */
    ret = sdskv_provider_handle_create(kvcl, svr_addr, mplex_id, &kvph);
    if(ret != 0)
    {
        printf("Rank %d, Error: sdskv_provider_handle_create()\n", rank);
        margo_addr_free(mid, svr_addr);
        sdskv_client_finalize(kvcl);
        margo_finalize(mid);
        return(-1);
    } else {
        printf("Rank %d: sdskv_provider_handle_create() success\n", rank);
    }
    PMPI_Barrier(MPI_COMM_WORLD);

    /* open the database */
    ret = sdskv_open(kvph, db_name, &db_id);
    if(ret == 0) {
        printf("Rank %d: Successfully opened database %s, id is %ld\n", rank, db_name, db_id);
        printf("Rank %d: db_id = %lu\n", rank, db_id);
    } else {
        printf("Rank %d: Error: could not open database %s\n", rank, db_name);
        printf("Rank %d: db_id = %lu\n", rank, db_id);
        printf("Rank %d: sdskv_open return value was %d\n", rank, ret);
        printf("Rank %d: error message was: %s\n", rank, sdskv_error_messages[ret]);
        printf("Rank %d: will not doing anything about error...\n", rank);
        //sdskv_provider_handle_release(kvph);
        //margo_addr_free(mid, svr_addr);
        //sdskv_client_finalize(kvcl);
        //margo_finalize(mid);
        //return(-1);
    }
    PMPI_Barrier(MPI_COMM_WORLD);

    return ret;
}
