#include "basic_task.h"

namespace auto_parallel
{

    const bool task::read_only = true;
    const bool task::read_write = false;

    task::task()
    {

    }

    task::task(message* dat)
    {
        data_v.push_back(dat);
        mods.push_back(read_write);
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
        mods.assign(mods.size(), read_write);
    }

    task::task(std::vector<message*>& mes_v, std::vector<bool>& mode_v)
    {
        data_v = mes_v;
        mods = mode_v;
    }

    task::~task()
    {

    }

}
