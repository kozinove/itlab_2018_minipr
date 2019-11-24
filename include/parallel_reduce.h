#ifndef __PARALLEL_REDUCE_H__
#define __PARALLEL_REDUCE_H__

#include <functional>
#include <mpi.h>
#include <iostream>
#include "parallel_vector.h"
  // std::function<int(int, int)>reduction - ?
  // std::function<int(int, int, const parallel_vector&, int)> func - ?
template<class Reduction>
int reduce_operation(int ans, const Reduction& reduction, int proccess_begin, int process_end, int proccess = 0) { // check it!
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int tmpans = ans;
    int n = 1;
    while(n < size)
        n *= 2;
    int tmprank = (rank-proccess)%n + (rank-proccess < 0?n:0);
    for(int i = 1; i < n; i = i * 2) {
        if(tmprank * 2*i < n) {
            MPI_Status status;
            int tmp;
            if(rank+n/(2*i) < size) {
                int sender = (tmprank + n/(2*i) + proccess)%n;
                if(sender >= size)
                    continue;
                MPI_Recv(&tmp, 1, MPI_INT, sender, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                tmpans = reduction(tmp, tmpans);
            }
        }
        else
        {
            MPI_Request request;
            int destination = (tmprank - n/(2*i) + proccess)%n;
            if(destination >= size)
                continue;
            MPI_Isend(&tmpans, 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &request);
            break;
        }        
    }
    return tmpans;
}

template<class Func, class Reduction>
int parallel_reduce(int l, int r, const parallel_vector& pv, int identity, const Func& func, const Reduction& reduction, int proccess = 0) {
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
    return reduce_operation(ans, reduction, proccess_begin, proccess_end, proccess);
}

#endif // __PARALLEL_REDUCE_H__