#include <iostream>
#include "parallel_vector.h"
//#include "parallel_for.h"
#include <mpi.h>

using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int n = atoi(argv[1]);
    parallel_vector pv(n);
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for(int i = 0; i < n; i++) {
        pv.set_elem(i, i);
    }
    double t1 = MPI_Wtime();
    int sum = 0;
    for(int i = 0; i < n; i++)
        sum += pv.get_elem(i);
    double t2 = MPI_Wtime();
    if(rank == 0)
        cout<<t2-t1;
    MPI_Finalize();
    return 0;
}