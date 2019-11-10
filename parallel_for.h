#ifndef __PARALLEL_FOR_H__
#define __PARALLEL_FOR_H__

#include <functional>
#include "parallel_vector.h"
#include "mpi.h"

//#define PARALLEL_SUM 0
//#define PARALLEL_MULT 1 // not ready yet

//parallel_sum
//parallel_mult
class parallel_for {
    int rank, size;
    int function_number;
public:
    parallel_for();
//    void operator()(const parallel_vector& pv, int num);
    void operator()(int l, int r, parallel_vector& pv, std::function<int(int)> f);
//    int get_ans(int proc = 0);
};

#endif // __PARALLEL_FOR_H__
