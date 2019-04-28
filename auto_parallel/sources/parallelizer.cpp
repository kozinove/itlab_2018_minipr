#include "parallelizer.h"

namespace auto_parallel
{

    const int parallelizer::main_proc = 0;

    parallelizer::parallelizer(int* argc, char*** argv)
    {
        int flag;
        MPI_Initialized(&flag);
        if (!flag)
            MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    }

    parallelizer::parallelizer(const task_graph& _tg, int* argc, char*** argv)
    {
        int flag;
        MPI_Initialized(&flag);
        if (!flag)
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
        std::map<int, task*> tmpr;
        std::map<int, message*> dmpr;
        unsigned i = 0;

        for (auto it = _tg.d_map.begin(); it != _tg.d_map.end(); ++it)
        {
            dmpr[(*it).second.id] = (*it).first;
        }

        for (auto it = dmpr.begin(); it != dmpr.end(); ++it, ++i)
        {
            dmp[(*it).second] = i;
            data_v[i].d = (*it).second;
            data_v[i].version = 0;
        }
        dmpr.clear();

        for (auto it = _tg.t_map.begin(); it != _tg.t_map.end(); ++it)
        {
            tmpr[(*it).second.id] = (*it).first;
        }

        i = 0;
        for (auto it = tmpr.begin(); it != tmpr.end(); ++it, ++i)
        {
            tmp[(*it).second] = i;
            task_v[i].t = (*it).second;
            task_v[i].parents = (*_tg.t_map.find((*it).second)).second.parents.size();
            if (task_v[i].parents == 0)
                ready_tasks.push(i);
        }
        tmpr.clear();

        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            unsigned j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            for (j = 0; j < task_v[i].t->data_v.size(); ++j)
                if (task_v[i].t->mods[j] == message::read_write)
                    task_v[i].data_id.push_back(dmp[task_v[i].t->data_v[j]]);
                else
                    task_v[i].const_data_id.push_back(dmp[task_v[i].t->data_v[j]]);
        }
    }

    void parallelizer::execution()
    {
        if (proc_size < 2)
        {
            MPI_Abort(MPI_COMM_WORLD, 1);
            throw -5;
        }
        if (proc_id == main_proc)
            master();
        else
            worker();
    }

    void parallelizer::master()
    {
        std::vector<std::vector<int>> versions(proc_size);
        for (std::vector<int>& i: versions)
            for (int j = 0; j < proc_size; ++j)
                i.push_back(j);
        std::set<int> ready_procs;
        std::queue<std::pair<int,int>> working_procs;
        for (int i = 0; i < proc_size; ++i)
            if (i != main_proc)
                ready_procs.insert(i);

        while (ready_tasks.size())
        {
            while (ready_tasks.size() && ready_procs.size())
            {
                int cur_t = ready_tasks.front();
                ready_tasks.pop();
                int cur_proc = *ready_procs.begin();
                ready_procs.erase(ready_procs.begin());
                control_task(cur_t, cur_proc);
                working_procs.push({cur_proc, cur_t});
            }
            while (working_procs.size())
            {
                std::pair<int,int> p = working_procs.front();
                working_procs.pop();
                wait_proc(p.second, p.first);
                ready_procs.insert(p.first);
            }
        }

        for (int i = 0; i < proc_size; ++i)
            if (i != main_proc)
                send_instruction(-1, i, 0);
    }

    void parallelizer::worker()
    {
        instruction cur_inst;
        bool exe = true;
        while(exe)
        {
            cur_inst = recv_instruction(main_proc);
            switch (cur_inst.n[0])
            {
            case 0:
                execute_task(cur_inst.n[1]);
                break;
            default:
                exe = false;
            }
        }
    }

    void parallelizer::control_task(int task_id, int proc)
    {

    }

    void parallelizer::wait_proc(int task_id, int proc)
    {

    }

    void parallelizer::execute_task(int task_id)
    {
        std::vector<int>& dv = task_v[task_id].data_id;
        MPI_Status status;
        int size;

        MPI_Probe(main_proc, 3, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &size);
        int* recv_d = new int[size];
        MPI_Recv(recv_d, size, MPI_INT, main_proc, 3, MPI_COMM_WORLD, &status);

        for (int i = 0; i < size; ++i)
        {
            data_v[recv_d[i]].d->recv(main_proc);
        }

        task_v[task_id].t->perform();

        for (size_t i = 0; i < dv.size(); ++i)
        {
            data_v[dv[i]].d->send(main_proc);
        }
        delete recv_d;
    }

    void parallelizer::send_instruction(int type, int proc, int info)
    {
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        instruction i;
        i.n[0] = type;
        i.n[1] = info;
        MPI_Send(i.n, 2, MPI_INT, proc, 1, MPI_COMM_WORLD);
    }

    parallelizer::instruction parallelizer::recv_instruction(int proc)
    {
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        MPI_Status status;
        instruction i;
        MPI_Recv(i.n, 2, MPI_INT, proc, 1, MPI_COMM_WORLD, &status);
        return i;
    }

    void parallelizer::send_ver_of_data(int did, int proc)
    {
        if ((did >= (int)data_v.size()) || (did < 0))
            throw -1;
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        MPI_Send(&data_v[did].version, 1, MPI_INT, proc, 2, MPI_COMM_WORLD);
    }

    int parallelizer::recv_ver_of_data(int did, int proc)
    {
        if ((did >= (int)data_v.size()) || (did < 0))
            throw -1;
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        MPI_Status status;
        int tmp;
        MPI_Recv(&tmp, 1, MPI_INT, proc, 2, MPI_COMM_WORLD, &status);
        return tmp;
    }

    void parallelizer::send_data(int did, int proc)
    {
        if ((did >= (int)data_v.size()) || (did < 0))
            throw -1;
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        data_v[did].d->send(proc);
        send_ver_of_data(did, proc);
    }

    void parallelizer::recv_data(int did, int proc)
    {
        if ((did >= (int)data_v.size()) || (did < 0))
            throw -1;
        if ((proc >= proc_size) || (proc < 0))
            throw -2;
        data_v[did].d->recv(proc);
        data_v[did].version = recv_ver_of_data(did, proc);
    }

}
