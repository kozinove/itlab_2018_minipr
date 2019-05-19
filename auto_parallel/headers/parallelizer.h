#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include <vector>
#include <queue>
#include <map>
#include <set>
#include "mpi.h"
#include "task_graph.h"
#include "it_queue.h"

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

        static double start_time;

        it_queue<std::pair<int, int>> ready_tasks;
        std::vector<t_info> task_v;
        std::vector<d_info> data_v;
        std::vector<int> top_versions;

        static std::map<std::pair<message*,int>, std::vector<MPI_Request>> requests;

        void master();
        void worker();
        void sequential_execution();
        // executes 1 chosen task
        void execute_task(int task_id);
        // main process wait execution of task by worker
        void wait_proc(int task_id, int proc, std::vector<std::set<int>>& v);
        // different sends
        void send_instruction(int type, int proc, int info = 0);
        instruction recv_instruction(int proc);

        void send_task_data(int tid, int proc, std::vector<std::set<int>>& ver);
        void recv_task_data(int tid, int proc);
        // give ids of workers cyclically
        void next_proc(int& proc);

    public:

        const static int main_proc; // 0

        parallelizer(int* argc = NULL, char*** argv = NULL);
        parallelizer(const task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();
        // use this as last argument in MPI_Isend/Irecv (mes = this)
        static MPI_Request* new_request(message* mes, int proc);

        static double get_start_time();

        int get_current_proc();

        void init(const task_graph& _tg); // initialization with tasks
        // starts execution of all tasks
        void execution();
    };

}

#endif // __PARALLELIZER_H__
