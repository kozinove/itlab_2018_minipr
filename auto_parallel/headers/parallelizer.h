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

        struct instruction
        {
            int n[2];
        };

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
            std::vector<int> const_data_id;
        };

        int proc_id;
        int proc_size;

        std::queue<int> ready_tasks;
        std::vector<t_info> task_v;
        std::vector<d_info> data_v;

        void master();
        void worker();

        void execute_task(int task_id);

        void send_instruction(int type, int proc, int info = 0);
        instruction recv_instruction(int proc);

        void send_ver_of_data(int did, int proc);
        int recv_ver_of_data(int did, int proc);

        void send_data(int did, int proc);
        void recv_data(int did, int proc);

    public:

        const static int main_proc;

        parallelizer(int mode, int* argc = NULL, char*** argv = NULL);
        parallelizer(int mode, const task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();

        void init(const task_graph& _tg);
        void execution();
    };

}

#endif // __PARALLELIZER_H__
