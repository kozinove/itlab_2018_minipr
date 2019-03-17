#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include "task_graph.h"
#include <vector>
#include <queue>
#include <map>
#include "mpi.h"

namespace auto_parallel
{

    class parallelizer
    {
    private:

        struct d_info
        {
            message* d;
            int version;
        };

        struct t_info
        {
            task* t;
            int parents;
            std::vector<int> childs;
            std::vector<int> data_id;
        };

        int proc_id;
        int proc_size;

        std::queue<int> ready_tasks;
        std::vector<t_info> task_v;
        std::vector<d_info> data_v;

    public:
        parallelizer(int mode, int* argc = NULL, char*** argv = NULL);
        parallelizer(int mode, const task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();
        void init(const task_graph& _tg);
        void execution();
    };

}

#endif // __PARALLELIZER_H__
