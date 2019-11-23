#ifndef __PARALLEL_REDUCE_H__
#define __PARALLEL_REDUCE_H__

#include <functional>
#include <mpi.h>
#include <iostream>
#include "parallel_vector.h"
  // std::function<int(int, int)>reduction - ?
  // std::function<int(int, int, const parallel_vector&, int)> func - ?
template<class Reduction>
int reduce_operation(int ans, const Reduction& reduction) { // check it!
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int tmpans = ans;
    int n = 1;
    while(n < size)
        n *= 2;
    for(int i = 1; i < n; i = i * 2) {
        if((rank+1) * 2*i <= n) {
            if(i == 1 && rank+n/2 >= size)
                continue;
            MPI_Status status;
            int tmp;
            MPI_Recv(&tmp, 1, MPI_INT, rank + n/(2*i), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tmpans = reduction(tmp, tmpans);
        }
        else
        {
            MPI_Send(&tmpans, 1, MPI_INT, rank - n/(2*i), 0, MPI_COMM_WORLD);
            break;
        }        
    }
    return tmpans;
}

template<class Func, class Reduction>
int parallel_reduce(int l, int r, const parallel_vector& pv, int identity, const Func& func, const Reduction& reduction) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int proccess_begin = pv.get_index_of_proccess(l);
    int proccess_end = pv.get_index_of_proccess(r);
    int ans = identity;
    if(rank >= proccess_begin && rank <= proccess_end) {
        int begin = 0, end = pv.get_portion();
        if(rank == proccess_begin)
            begin = pv.get_index_of_element(l);
        if(rank == proccess_end)
            end = pv.get_index_of_element(r);
        ans = func(begin, end, identity);
    }
    //return ans;
    // std::cout<<ans<<" "<<l<<" "<<r<<" | "<<proccess_begin<<" "<<proccess_end<<"\n";
    return reduce_operation(ans, reduction);
}

#endif // __PARALLEL_REDUCE_H__