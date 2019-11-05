#include <iostream>
#include <mpi.h>
#include "parallel_vector.h"
#include "parallel_for.h"

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);
    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    parallel_for pf;
    int n = atoi(argv[1]);
    parallel_vector pv(n);
    for(int i = 0; i < n; i++)
        pv.set_elem(i, i);
    double t1 = MPI_Wtime();
    pf(pv, PARALLEL_SUM);
    int ans = pf.get_ans(0);
    // if(rank == 0)
    //     std::cout<<ans<<"\n";
    double t2 = MPI_Wtime();
    if(rank == 0)
        std::cout<<t2-t1;
    MPI_Finalize();
}