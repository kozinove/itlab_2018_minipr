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

        class instruction: public sendable
        {
        public:

            enum class cmd: int
            {
                UNDEFINED, END, MES_SEND, MES_RECV, MES_CREATE,
                MES_P_CREATE, TASK_EXE, TASK_CREATE, TASK_RES
            };

        private:

            std::vector<int> v;
            cmd previous;
            int prev_pos;

            void add_cmd(cmd id);

        public:

            instruction();
            ~instruction();

            void send(const sender& se);
            void recv(const receiver& re);

            int& operator[](size_t n);
            const int& operator[](size_t n) const;

            size_t size();

            void clear();

            void add_end();
            void add_message_sending(int id);
            void add_message_receiving(int id);
            void add_message_creation(int id, int type);
            void add_message_part_creation(int id, int type, int source);
            void add_task_execution(int id);
            void add_task_creation(int id, int type, std::vector<int> data, std::vector<int> c_data);
            void add_task_result(int id, task_environment& env);

        };

        struct d_info
        {
            message* d;
            int parent;
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

        void wait_proc(int task_id, int proc, std::vector<std::set<int>>& v);

        void send_task_data(int tid, int proc, std::vector<std::set<int>>& ver);
        void recv_task_data(int tid, int proc);

        void next_proc(int& proc);

        void create_message(int id, int type, int proc);
        void create_part(int id, int type, int source, int proc);
        int create_task(int* inst);
        void execute_task(int id);

        

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
