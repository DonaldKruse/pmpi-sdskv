#ifndef PMPI_LIB_H
#define PMPI_LIB_H

#include <margo.h>
#include "../include/sdskv-client.h"
#include "../include/sdskv-server.h"
#include "../include/sdskv-common.h"


char* get_key(int rank, const char* postfix);

int MPI_Init(int *argc, char ***argv);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm);

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
             int source, int tag, MPI_Comm comm, MPI_Status *status);

int MPI_Finalize();


#endif
