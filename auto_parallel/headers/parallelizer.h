#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include "task_graph.h"
#include <vector>
#include <queue>
#include "mpi.h"

namespace auto_parallel
{

    class parallelizer
    {
    private:
        task_graph tg;
        int proc_id;
        int proc_size;
        std::queue<task*> task_v;
        std::vector<message*> data_v;
    public:
        parallelizer(int mode, task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();
        void execution();
    };

}

#endif // __PARALLELIZER_H__
