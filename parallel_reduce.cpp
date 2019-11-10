#include "parallel_reduce.h"

parallel_reduce::parallel_reduce() {
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

int parallel_reduce::operator()(int l, int r, const parallel_vector& pv, int identity,
             std::function<int(int, int, const parallel_vector&, int)>func, std::function<int(int, int)>reduction) {
    int proccess_begin = pv.get_index_of_proccess(l);
    int proccess_end = pv.get_index_of_proccess(r);
    int ans = identity;
    if(rank >= proccess_begin && rank <= proccess_end) {
        int begin = 0, end = pv.get_portion();
        if(rank == proccess_begin)
            begin = pv.get_index_of_element(l);
        if(rank == proccess_end)
            end = pv.get_index_of_element(r);
        ans = func(begin, end, pv, 0);
    }
    //return ans;
    return reduce_operation(ans, reduction);
}

int parallel_reduce::reduce_operation(int ans, std::function<int(int, int)>reduction) { // check it!
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