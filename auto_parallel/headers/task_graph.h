#ifndef __TASK_GRAPH_H__
#define __TASK_GRAPH_H__

#include "basic_task.h"
#include <vector>

namespace auto_parallel
{

    class task_graph
    {
    protected:
        struct d_id
        {
            int id;
            message* pm;
            int version;
        };
        struct t_id
        {
            int id;
            task* pt;
            int end_count;
        };
        std::vector<t_id> task_v;
        std::vector<d_id> data_v;
    public:
        task_graph();
        task_graph(task_graph& _tg);
        void add_task(task* t);
        void add_data(message* m);
    };

}

#endif // __TASK_GRAPH_H__
