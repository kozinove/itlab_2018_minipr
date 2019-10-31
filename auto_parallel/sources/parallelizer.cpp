#include "parallelizer.h"

namespace auto_parallel
{

    parallelizer::instruction::instruction(): sendable()
    {
        previous = cmd::UNDEFINED;
        prev_pos = -1;
    }

    parallelizer::instruction::~instruction()
    { }

    void parallelizer::instruction::send(const sender& se)
    { se.send(v.data(), v.size(), MPI_INT); }

    void parallelizer::instruction::recv(const receiver& re)
    {
        v.resize(re.probe(MPI_INT));
        re.recv(v.data(), v.size(), MPI_INT);
    }

    int& parallelizer::instruction::operator[](size_t n)
    { return v[n]; }

    const int& parallelizer::instruction::operator[](size_t n) const
    { return v[n]; }

    size_t parallelizer::instruction::size()
    { return v.size(); }

    void parallelizer::instruction::clear()
    { v.clear(); }

    void parallelizer::instruction::add_cmd(cmd id)
    {
        if (id == previous)
            ++v[prev_pos + 1];
        else
        {
            previous = id;
            prev_pos = v.size();
            v.push_back(static_cast<int>(id));
            v.push_back(1);
        }
    }

    void parallelizer::instruction::add_end()
    { add_cmd(cmd::END); }

    void parallelizer::instruction::add_message_sending(int id)
    {
        add_cmd(cmd::MES_SEND);
        v.push_back(id);
    }

    void parallelizer::instruction::add_message_receiving(int id)
    {
        add_cmd(cmd::MES_RECV);
        v.push_back(id);
    }

    void parallelizer::instruction::add_message_creation(int id, int type)
    {
        add_cmd(cmd::MES_CREATE);
        v.push_back(id);
        v.push_back(type);
    }

    void parallelizer::instruction::add_message_part_creation(int id, int type, int source)
    {
        add_cmd(cmd::MES_P_CREATE);
        v.push_back(id);
        v.push_back(type);
        v.push_back(source);
    }

    void parallelizer::instruction::add_task_execution(int id)
    {
        add_cmd(cmd::TASK_EXE);
        v.push_back(id);
    }

    void parallelizer::instruction::add_task_creation(int id, int type, std::vector<int> data, std::vector<int> c_data)
    {
        add_cmd(cmd::TASK_CREATE);
        v.push_back(id);
        v.push_back(type);
        v.push_back(data.size());
        for (int i: data)
            v.push_back(i);
        v.push_back(c_data.size());
        for (int i: c_data)
            v.push_back(i);
    }

    void parallelizer::instruction::add_task_result(int id, task_environment& env)
    {
        add_cmd(cmd::TASK_RES);
        std::vector<task_environment::message_data>& md = env.get_c_messages();
        std::vector<task_environment::message_part_data>& mpd = env.get_c_parts();
        std::vector<task_environment::task_data>& td = env.get_c_tasks();

        v.push_back(id);
        v.push_back(md.size());
        for (int i = 0; i < md.size(); ++i)
            v.push_back(md[i].type);
        v.push_back(mpd.size());
        for (int i = 0; i < mpd.size(); ++i)
        {
            v.push_back(mpd[i].type);
            v.push_back(mpd[i].sourse.id);
            v.push_back(static_cast<int>(mpd[i].sourse.ms));
        }
        v.push_back(td.size());
        for (int i = 0; i < td.size(); ++i)
        {
            v.push_back(td[i].type);
            v.push_back(td[i].ti->data.size());
            for (task_environment::mes_id j: td[i].ti->data)
            {
                v.push_back(j.id);
                v.push_back(static_cast<int>(j.ms));
            }
            v.push_back(td[i].ti->c_data.size());
            for (task_environment::mes_id j: td[i].ti->c_data)
            {
                v.push_back(j.id);
                v.push_back(static_cast<int>(j.ms));
            }
        }
    }

    const int parallelizer::main_proc = 0;

    parallelizer::parallelizer(int* argc, char*** argv): parallel_engine(argc, argv), comm(MPI_COMM_WORLD), instr_comm(comm)
    { }

    parallelizer::parallelizer(task_graph& _tg, int* argc, char*** argv): parallel_engine(argc, argv), comm(MPI_COMM_WORLD), instr_comm(comm)
    { init(_tg); }

    parallelizer::~parallelizer()
    { }

    void parallelizer::init(task_graph& _tg)
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
            dmpr[(*it).second.id] = (*it).first;

        for (auto it = dmpr.begin(); it != dmpr.end(); ++it, ++i)
        {
            dmp[(*it).second] = i;
            data_v[i].d = (*it).second;
            data_v[i].version = 0;
        }
        dmpr.clear();

        for (auto it = _tg.t_map.begin(); it != _tg.t_map.end(); ++it)
            tmpr[(*it).second.id] = (*it).first;

        i = 0;
        for (auto it = tmpr.begin(); it != tmpr.end(); ++it, ++i)
        {
            tmp[(*it).second] = i;
            task_v[i].t = (*it).second;
            task_v[i].parents = int((*_tg.t_map.find((*it).second)).second.parents.size());
            if (task_v[i].parents == 0)
                ready_tasks.push({i, -1});
        }
        tmpr.clear();

        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            size_t j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            for (j = 0; j < task_v[i].t->data.size(); ++j)
                task_v[i].data_id.push_back(dmp[task_v[i].t->data[j]]);
            for (j = 0; j < task_v[i].t->c_data.size(); ++j)
            {
                message* t = const_cast<message*>(task_v[i].t->c_data[j]);
                task_v[i].const_data_id.push_back(dmp[t]);
            }
        }
        _tg.clear();

        top_versions.resize(data_v.size());
        top_versions.assign(data_v.size(), 0);
    }

    void parallelizer::execution()
    {
        if (comm.get_size() < 2)
            sequential_execution();
        else if (comm.get_rank() == main_proc)
            master();
        else
            worker();
    }

    void parallelizer::sequential_execution()
    {
        while(ready_tasks.size())
        {
            std::pair<int, int> p = ready_tasks.front();
            ready_tasks.pop();
            task_v[p.first].t->perform();
            for (int i: task_v[p.first].childs)
            {
                --task_v[i].parents;
                if (task_v[i].parents <= 0)
                    ready_tasks.push({i, -1});
            }
        }
    }

    void parallelizer::master()
    {
        std::vector<std::set<int>> versions(data_v.size());
        for (std::set<int>& i: versions)
            for (int j = 0; j < comm.get_size(); ++j)
                i.insert(j);

        std::queue<std::pair<int,int>> working_procs;

        std::set<int> ready_procs;
        for (int i = 0; i < comm.get_size(); ++i)
            if (i != main_proc)
                ready_procs.insert(i);

        int start_task = 0;
        int my_task = -1;
        int cur_proc = 0;

        while (ready_tasks.size())
        {

            for (int i = start_task; i < ready_tasks.size(); ++i)
            {
                if (ready_tasks[i].second == -1)
                {
                    if (cur_proc != main_proc)
                    {
                        send_instruction(1, cur_proc, ready_tasks[i].first);
                        send_task_data(ready_tasks[i].first, cur_proc, versions);
                    }
                    ready_tasks[i].second = cur_proc;
                    next_proc(cur_proc);
                }
                ++start_task;
            }
            start_task -= (int)ready_procs.size() + 1;

            while (ready_tasks.size())
            {
                std::pair<int,int> cur_t = ready_tasks.front();
                if (cur_t.second == main_proc)
                {
                    if (my_task != -1)
                        break;
                    ready_tasks.pop();
                    my_task = cur_t.first;
                    continue;
                }
                if (ready_procs.find(cur_t.second) == ready_procs.end())
                    break;
                ready_procs.erase(cur_t.second);
                ready_tasks.pop();
                send_instruction(0, cur_t.second, cur_t.first);
                working_procs.push({cur_t.second, cur_t.first});
            }
            start_task += (int)ready_procs.size();

            if (my_task != -1)
            {
                task_v[my_task].t->perform();
                std::vector<int>& d = task_v[my_task].data_id;
                for (int i = 0; i < d.size(); ++i)
                {
                    top_versions[d[i]]++;
                    versions[d[i]].clear();
                    versions[d[i]].insert(main_proc);
                }
                for (int i: task_v[my_task].childs)
                {
                    --task_v[i].parents;
                    if (task_v[i].parents <= 0)
                        ready_tasks.push({i, -1});
                }
                my_task = -1;
            }
            else
                start_task += 1;

            while (working_procs.size())
            {
                std::pair<int,int> p = working_procs.front();
                working_procs.pop();
                wait_proc(p.second, p.first, versions);
                ready_procs.insert(p.first);
            }
        }

        instruction end;
        end.add_end();
        for (int i = 1; i < instr_comm.get_size(); ++i)
            instr_comm.send(&end, i);
    }

    void parallelizer::worker()
    {
        instruction cur_inst;
        while(1)
        {
            instr_comm.recv(&cur_inst, main_proc);
            int j = 0;

            while (j < cur_inst.size())
            {
                int cur_i_pos = j;
                j += 2;
                switch (static_cast<instruction::cmd>(cur_inst[cur_i_pos]))
                {
                case instruction::cmd::MES_SEND:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                        comm.send(data_v[cur_inst[j++]].d, main_proc);
                    break;

                case instruction::cmd::MES_RECV:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                        comm.recv(data_v[cur_inst[j++]].d, main_proc);
                    break;

                case instruction::cmd::MES_CREATE:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                    {
                        create_message(cur_inst[j], cur_inst[j + 1], main_proc);
                        j += 2;
                    }
                    break;

                case instruction::cmd::MES_P_CREATE:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                    {
                        create_part(cur_inst[j], cur_inst[j + 1], cur_inst[j + 2], main_proc);
                        j += 3;
                    }
                    break;

                case instruction::cmd::TASK_CREATE:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                        j += create_task(&cur_inst[j]);
                    break;

                case instruction::cmd::TASK_EXE:

                    for (int i = 0; i < cur_inst[cur_i_pos + 1]; ++i)
                        execute_task(cur_inst[j++]);
                    break;

                case instruction::cmd::END:

                    goto end;

                default:

                    MPI_Abort(instr_comm.get_comm(), 234);
                }
            }
        }
        end:;
    }

    void parallelizer::create_message(int id, int type, int proc)
    {
        message::init_info_base* iib = message_factory::get_info(type);
        instr_comm.recv(iib, proc);
        if (data_v.size() <= id)
            data_v.resize(id + 1);
        data_v[id] = {message_factory::get(type, iib), -1, 0};
    }

    void parallelizer::create_part(int id, int type, int source, int proc)
    {
        message::part_info_base* pib = message_factory::get_part_info(type);
        instr_comm.recv(pib, proc);
        message* src = data_v[source].d;
        if (data_v.size() <= id)
            data_v.resize(id + 1);
        data_v[id] = {message_factory::get_part(type, src, pib), source, data_v[source].version};
    }

    int parallelizer::create_task(int* inst)
    {
        int ret = 4;
        int sz = inst[2];
        ret += sz;
        int* p = inst + 3;

        std::vector<message*> data;
        data.reserve(sz);
        std::vector<const message*> c_data;
        c_data.reserve(sz);
        std::vector<int> data_id;
        data_id.reserve(sz);
        std::vector<int> const_data_id;
        const_data_id.reserve(sz);

        for (int i = 0; i < sz; ++i)
        {
            data.push_back(data_v[p[i]].d);
            data_id.push_back(p[i]);
        }

        p += sz + 1;
        sz = *(p - 1);
        ret += sz;

        for (int i = 0; i < sz; ++i)
        {
            c_data.push_back(data_v[p[i]].d);
            const_data_id.push_back(p[i]);
        }

        task_v[inst[0]] = {task_factory::get(inst[1], data, c_data), 0, std::vector<int>(), data_id, const_data_id};
        return ret;
    }

    void parallelizer::execute_task(int task_id)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i = 0; i < cd.size(); ++i)
            data_v[cd[i]].d->wait_requests();

        task_environment::task_info ti;
        for (int i: d)
            ti.data.push_back({i, task_environment::message_source::TASK_ARG});
        for (int i: cd)
            ti.c_data.push_back({i, task_environment::message_source::TASK_ARG_C});

        task_environment::task_data td = {-1, &ti};

        task_environment env(std::move(td));
        task_v[task_id].t->perform(env);

        instruction res;
        res.add_task_result(task_id, env);

        instr_comm.send(&res, main_proc);
        for (int i = 0; i < env.get_c_messages.size(); ++i)
            instr_comm.send(env.get_c_messages[i].iib, main_proc);
        for (int i = 0; i < env.get_c_parts.size(); ++i)
            instr_comm.send(env.get_c_parts.pib, main_proc);

        for (size_t i = 0; i < d.size(); ++i)
            comm.send(data_v[d[i]].d, main_proc);

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();
    }

    void parallelizer::send_task_data(int tid, int proc, std::vector<std::set<int>>& ver)
    {
        std::vector<int>& d = task_v[tid].data_id;
        std::vector<int>& cd = task_v[tid].const_data_id;
        std::vector<int> low_versions;

        for (int i = 0; i < d.size(); ++i)
        {
            if (ver[d[i]].find(proc) == ver[d[i]].end())
            {
                ver[d[i]].insert(proc);
                low_versions.push_back(d[i]);
            }
        }
        for (int i = 0; i < cd.size(); ++i)
        {
            if (ver[cd[i]].find(proc) == ver[cd[i]].end())
            {
                ver[cd[i]].insert(proc);
                low_versions.push_back(cd[i]);
            }
        }

        MPI_Send(low_versions.data(), low_versions.size(), MPI_INT, proc, 3, instr_comm.get_comm());

        for (int i = 0; i < low_versions.size(); ++i)
            comm.send(data_v[low_versions[i]].d, proc);

    }

    void parallelizer::wait_proc(int task_id, int proc, std::vector<std::set<int>>& v)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i = 0; i < cd.size(); ++i)
            data_v[cd[i]].d->wait_requests();

        for (int i = 0; i < d.size(); ++i)
        {
            comm.recv(data_v[d[i]].d, proc);
            top_versions[d[i]]++;
            v[d[i]].clear();
            v[d[i]].insert(proc);
            v[d[i]].insert(main_proc);
        }

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i: task_v[task_id].childs)
        {
            --task_v[i].parents;
            if (task_v[i].parents <= 0)
                ready_tasks.push({i, -1});
        }
    }

    void parallelizer::recv_task_data(int tid, int proc)
    {
        MPI_Status status;
        int size;

        MPI_Probe(proc, 3, instr_comm.get_comm(), &status);
        MPI_Get_count(&status, MPI_INT, &size);
        int* recv_d = new int[size];
        MPI_Recv(recv_d, size, MPI_INT, proc, 3, instr_comm.get_comm(), &status);

        for (int i = 0; i < size; ++i)
            comm.recv(data_v[recv_d[i]].d, proc);

        delete[] recv_d;
    }

    void parallelizer::next_proc(int& proc)
    { proc = (1 + proc) % comm.get_size(); }

    int parallelizer::get_current_proc()
    { return comm.get_rank(); }

}
