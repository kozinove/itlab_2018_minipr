#ifndef __PARALLEL_REDUCE_H__
#define __PARALLEL_REDUCE_H__

#include <functional>
#include <mpi.h>

#include "parallel_vector.h"
class parallel_reduce {
    int size, rank;
public:
    parallel_reduce();
    int operator()(int l, int r, const parallel_vector& pv, int identity, std::function<int(int, int, const parallel_vector&, int)>func, std::function<int(int, int)>reduction);
private:
    int reduce_operation(int ans, std::function<int(int, int)>reduction);
};

#endif // __PARALLEL_REDUCE_H__