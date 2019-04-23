#include "basic_task.h"

namespace auto_parallel
{

    task::task()
    {

    }

    task::task(message* dat)
    {
        data_v.push_back(dat);
        mods.push_back(message::read_write);
    }

    task::task(message* dat, bool mode)
    {
        data_v.push_back(dat);
        mods.push_back(mode);
    }

    task::task(std::vector<message*>& mes_v)
    {
        data_v = mes_v;
        mods.resize(data_v.size());
        mods.assign(mods.size(), message::read_write);
    }

    task::task(std::vector<message*>& mes_v, std::vector<bool>& mode_v)
    {
        if (mes_v.size() != mode_v.size())
            throw -4;
        data_v = mes_v;
        mods = mode_v;
    }

    task::~task()
    {

    }

}
