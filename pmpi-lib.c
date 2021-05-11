#include <assert.h>
#include <mpi.h>
#include <stdio.h>
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

//static const char* keys[] = {"MPI_Init",
//                             "MPI_Send",
//                             "MPI_Isend",
//                             "MPI_Recv",
//                             "MPI_Finalize"};
//static const unsigned total_keys = 5;



/** 
 * The keys are generated from the rank and these postfixes.
 * A valid key would look like "<rank>:<postfix>".
 * Keys are generated in get_key().
 * Eventually it would be nice to either define these
 * postfixes in the header fileor read in a file of 
 * postfixes during initialization.
 * This file could include threshold values as well.
 *
 * Right now this array is not really being used.
 */
static const char* key_postfix[] = {
    "MPI_Isend",
    "MPI_Send",
    "MPI_Recv"
};
static const unsigned total_keys = 3;


/**
 * These are the counters used to determine the frequencey of puts into
 * the database
 */
static unsigned long num_recv = 0;
static unsigned long tot_recv = 0;
unsigned int global_threshold_recv;

static unsigned long num_isend = 0;
static unsigned long tot_isend = 0;
unsigned int global_threshold_isend;

static unsigned long num_send = 0;
static unsigned long tot_send = 0;
unsigned int global_threshold_send;


char* get_key(int rank, const char* postfix) {
    // we need to get the number of characters for the key.
    // A key should look like
    // "<rank>:<keypostfix>"
    // where <rank> is the integer value of the MPI rank doing the put
    // into the database and keypostfix is the postfix identifying which
    // MPI routine was called.
    // e.g. if rank 2 calls MPI_Isend, the key would then be
    //     "2:MPI_Isend"
    unsigned numchars= 1; // number of chars for the key, inclu
    numchars += strlen(postfix);

    if (rank < 10) {
	numchars += 1;
    } else if (rank < 100) {
	numchars += 2;
    } else if (rank < 1000) {
	numchars += 3;
    } else {
	printf("Number of ranks is >= 1000. Augment this function, otherwise quitting...\n");
	exit(1);
    }
    char* key = (char*) malloc((numchars+1)*sizeof(char)); // the +1 is for null terminator
    if (!key) {
	printf("error get_key(): malloc unsuccessful. Quitting...\n");
	exit(1);
    }
    int err;
    err = snprintf(key, numchars+1,"%u:%s",rank, postfix);
    if (err < 0) {
	printf("error get_key(): snprinf returned %d (error)\n", err);
	free(key);
	return NULL;
    }
    key[numchars] = 0;
    return key;
} 

int MPI_Init(int *argc, char ***argv)
{
    int mpi_init_ret;
    int ssg_ret;
    const char* filename = "pmpi-params.txt";
    int ret;
    int rank;
    int world_size;


    if(*argc != 5)
    {
        fprintf(stderr, "MPI_Init: Usage: %s <sdskv_server_addr> <mplex_id> <db_name> <num_keys>\n",
                (*argv)[0]);
        fprintf(stderr, "  Example: %s tcp://localhost:1234 1 foo 1000\n", (*argv)[0]);
        return(-1);
    }
    
    /**
     * Eventually this file open stuff needs to move somewhere else or be wrapped.
     * See get_params() in pmpi-common.c.
     */

    mpi_init_ret = PMPI_Init(argc, argv);


    PMPI_Comm_size(MPI_COMM_WORLD, &world_size);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
    /* Write all the keys to a file so that it is easier to know them a priori */
    if (rank == 0) {
	int postfix;
	int r;
	char* key = NULL;
	
	FILE* fp = fopen("keys.txt", "w+");
	if (fp == NULL) {
	    printf("Could not open 'keys.txt'. Quitting...\n");
	    exit(1);
	}

	for (postfix = 0; postfix < total_keys; postfix++) {
	    for (r = 0; r < world_size; r++) {
		key = get_key(r, key_postfix[postfix]); 
		fputs(key, fp);
		fputs("\n", fp);
		free(key);
	    }
	}
	fclose(fp);
    }
    PMPI_Barrier(MPI_COMM_WORLD);

    // TODO - there really should broadcast a struct..
    // this will work for now, but obviously it won't scale well.
    /* Get threshold parameters */
    if (rank == 0) {
	get_parameters();
    }
    ret = PMPI_Bcast(&global_threshold_recv, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    if (ret != MPI_SUCCESS) {
	printf("Could not broadcast global_threshold_recv! Quitting...\n");
	PMPI_Finalize();
	exit(1);
    }
    if (rank != 0)
	printf("Rank %d got global_threshold_recv (%u)\n", rank, global_threshold_recv);

    ret = PMPI_Bcast(&global_threshold_isend, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    if (ret != MPI_SUCCESS) {
	printf("Could not broadcast global_threshold_isend! Quitting...\n");
	PMPI_Finalize();
	exit(1);
    }
    if (rank != 0)
	printf("Rank %d got global_threshold_isend (%u)\n", rank, global_threshold_recv);
    ret = PMPI_Bcast(&global_threshold_send, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    if (ret != MPI_SUCCESS) {
	printf("Could not broadcast global_threshold_send! Quitting...\n");
	PMPI_Finalize();
	exit(1);
    }
    if (rank != 0)
	printf("Rank %d got global_threshold_send (%u)\n", rank, global_threshold_recv);

    PMPI_Barrier(MPI_COMM_WORLD);
    init_margo_open_db_check_error(argc, argv);


    PMPI_Barrier(MPI_COMM_WORLD);
    return mpi_init_ret;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm)
{

    static const char* key_postfix = "MPI_Send";
    static const hg_size_t dsize = sizeof(unsigned long);
    static char* key = NULL;
    int rank;
    int ret;
    if (key == NULL) {
    	PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
	key = get_key(rank, key_postfix);
    }
    ret = PMPI_Send(buf, count, datatype, dest, tag, comm);
    update_count_sdskv_put(&num_send, &tot_send, global_threshold_send, (const) key, dsize);
    return ret;
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request *request)
{
    static const char* key_postfix = "MPI_Isend";
    static const hg_size_t dsize = sizeof(unsigned long);
    static char* key = NULL;
    int rank;
    int ret;
    if (key == NULL) {
    	PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
	key = get_key(rank, key_postfix);
    }
    ret = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
    update_count_sdskv_put(&num_isend, &tot_isend, global_threshold_isend, (const) key, dsize);
    return ret;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
             int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    static const char* key_postfix = "MPI_Recv";
    static const hg_size_t dsize = sizeof(unsigned long);
    static char* key = NULL;
    int ret;
    int rank;
    if (key == NULL) {
    	PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
	key = get_key(rank, key_postfix);
    }
    ret = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
    update_count_sdskv_put(&num_recv, &tot_recv, global_threshold_recv, (const) key, dsize);
    return ret;
}

int MPI_Finalize()
{
    int pmpi_finalize_ret = 0;

    int rank;
    PMPI_Barrier(MPI_COMM_WORLD);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);


    // This block should eventually be removed and each
    // MPI routine should have its own sdskv_put_check_err
    //unsigned int data[num_keys] = {num_init, num_send, num_isend, num_recv, num_finalize};
    //unsigned long data[total_keys] = {num_init, num_send, num_isend, num_recv, num_finalize};
 										   
    unsigned i;
    unsigned ksize = 0;
    unsigned dsize = 0;
    //for (i = 0; i < total_keys; i++) {
    //    ksize = sizeof(char)*strlen(keys[i]);
    //    dsize = sizeof(unsigned long);
    //	unsigned long global_data = 0;
    //	PMPI_Reduce(&(data[i]), &global_data, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    //	if (rank == 0) {
    //	    printf("Rank %d: global_data is %lu for key %s\n", 0, global_data, keys[i]);
    //	    sdskv_put_check_err( (const void*) keys[i], ksize,
    //	    			 (const void*) &global_data, dsize);
    //	}
    //	PMPI_Barrier(MPI_COMM_WORLD);
    //}

    pmpi_finalize_ret = PMPI_Finalize();
    return pmpi_finalize_ret;
}
