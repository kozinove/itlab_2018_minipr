#include "basic_task.h"

namespace auto_parallel
{
    task::task()
    {

    }

    task::task(message* dat)
    {
        data_v.push_back(dat);
    }

    task::task(std::vector<message*>& mes_v)
    {
        data_v = mes_v;
    }

    task::~task()
    {

    }

}
