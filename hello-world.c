#include <mpi.h>
#include <stdio.h>

int main(int argv, char** argc) {
    int world_size;
    int rank;
    char proc_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Get_processor_name(proc_name, &name_len);

    printf("Hello from proc %s, rank %d out of %d procs\n", 
	   proc_name, 
	   rank, 
	   world_size);

    MPI_Finalize();

    return 0;
}
	   
