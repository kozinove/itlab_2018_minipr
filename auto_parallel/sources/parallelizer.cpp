#include "parallelizer.h"

namespace auto_parallel
{

    parallelizer::parallelizer(int mode, int* argc, char*** argv)
    {
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    }

    parallelizer::parallelizer(int mode, const task_graph& _tg, int* argc, char*** argv)
    {
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
        init(_tg);
    }

    parallelizer::~parallelizer()
    {
        MPI_Finalize();
    }

    void parallelizer::init(const task_graph& _tg)
    {
        while (ready_tasks.size())
            ready_tasks.pop();
        task_v.clear();
        data_v.clear();
        task_v.resize(_tg.t_map.size());
        data_v.resize(_tg.d_map.size());
        std::map<task*, int> tmp;
        std::map<message*, int> dmp;
        unsigned i = 0;
        for (auto it = _tg.d_map.begin(); it != _tg.d_map.end(); ++it, ++i)
        {
            dmp[(*it).first] = i;
            data_v[i].d = (*it).first;
            data_v[i].version = 0;
        }
        i = 0;
        for (auto it = _tg.t_map.begin(); it != _tg.t_map.end(); ++it, ++i)
        {
            tmp[(*it).first] = i;
            task_v[i].t = (*it).first;
            task_v[i].parents = (*it).second.parents.size();
            if ((*it).second.parents.empty())
                ready_tasks.push(i);
        }
        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            unsigned j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            task_v[i].data_id.resize(task_v[i].t->data_v.size());
            for (j = 0; j < task_v[i].t->data_v.size(); ++j)
                task_v[i].data_id[j] = dmp[task_v[i].t->data_v[j]];
        }
    }

}
