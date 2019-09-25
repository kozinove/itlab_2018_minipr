#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include <vector>
#include <queue>
#include <map>
#include <set>
#include "mpi.h"
#include "parallel_core.h"
#include "task_graph.h"
#include "intracomm.h"
#include "it_queue.h"

namespace auto_parallel
{

    class parallelizer: public parallel_engine
    {
    private:

        struct instruction
        { int n[2]; };

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

        intracomm comm;
        intracomm instr_comm;

        it_queue<std::pair<int, int>> ready_tasks;
        std::vector<t_info> task_v;
        std::vector<d_info> data_v;

        std::vector<int> top_versions;

        void master();
        void worker();
        void sequential_execution();

        void execute_task(int task_id);

        void wait_proc(int task_id, int proc, std::vector<std::set<int>>& v);

        void send_instruction(int type, int proc, int info = 0);
        instruction recv_instruction(int proc);

        void send_task_data(int tid, int proc, std::vector<std::set<int>>& ver);
        void recv_task_data(int tid, int proc);

        void next_proc(int& proc);

    public:

        const static int main_proc;

        parallelizer(int* argc = NULL, char*** argv = NULL);
        parallelizer(task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();

        int get_current_proc();

        void init(task_graph& _tg);

        void execution();

    };

}

#endif // __PARALLELIZER_H__
