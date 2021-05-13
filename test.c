#include <stdio.h>
#include <mpi.h>


int main(int argc, char** argv)
{
    int rank;
    int numranks;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);

    printf("Hello from rank %d out of %d\n", rank, numranks);

    MPI_Finalize();


    return 0;
}
