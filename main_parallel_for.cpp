#include <iostream>
#include <mpi.h>
#include "parallel_vector.h"
#include "parallel_for.h"
#include "parallel_reduce.h"

int func(int l, int r, const parallel_vector& pv, int identity) {
    int ans = identity;
    for(int i = l; i < r; i++)
        ans+=pv.get_elem_proc(i);
    return ans;
}

int reduction(int a, int b)
{
    return a + b;
}

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);
    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    parallel_for pf;
    parallel_reduce pr;
    int n;
    if(argc == 1)
        n = 10;
    else
        n = atoi(argv[1]);
    parallel_vector pv(n);
    for(int i = 0; i < n; i++)
        pv.set_elem(i, i);
    for(int i = 0; i < n; i++) {
        int ans = pv.get_elem(i);
        if(rank == 0)
            std::cout<<ans<<" ";
    }
    if(rank == 0)
        std::cout<<"\n";
    pf(3, 4, pv, [](int a){return a + 5;});
    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < n; i++) {
        int ans = pv.get_elem(i);
        if(rank == 0)
            std::cout<<ans<<" ";
    }
    if(rank == 0)
        std::cout<<"\n";
    int ans = pr(0, n, pv, 0, func, reduction);
    if(rank == 0)
        std::cout<<"sum of vector = "<<ans<<"\n";
    MPI_Finalize();
}
