#ifndef PMPI_COMMON_H
#define PMPI_COMMON_H

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ssg.h>
#include <ssg-mpi.h>


#include "pmpi-lib.h"

#include <margo.h>
#include "../include/sdskv-client.h"
#include "../include/sdskv-server.h"
#include "../include/sdskv-common.h"


unsigned int global_threshold_recv;
unsigned int global_threshold_isend;
unsigned int global_threshold_send;
unsigned int global_threshold_reduce;


void get_keys_from_file(const char* filename, char*** keylist, unsigned* numkeys);

void free_keylist(char*** keylist, unsigned numkeys);

void update_count_sdskv_put(unsigned *local_count,
			    unsigned *total_count,
			    const unsigned threshold,
			    const char* key,
			    const hg_size_t dsize);


void get_parameters();

static void my_membership_update_cb(void* uargs,
                                    ssg_member_id_t member_id,
                                    ssg_member_update_type_t update_type);
/**
 * Puts a key-value pair in the sdskv database and checks
 * for errors. If any errors happen during the sdskv_put,
 * this function shuts down and frees the server, and returns
 * -1, otherwise returns the return value from sdskv_put.
 */
int sdskv_put_check_err(const void *key, hg_size_t ksize,
                        const void *value, hg_size_t vsize);

int sdskv_shutdown_service_cleanup();

int init_margo_open_db_check_error(int* argc, char*** argv);


#endif
