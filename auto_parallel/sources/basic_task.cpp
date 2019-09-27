#include "basic_task.h"

namespace auto_parallel
{

    task::task(std::vector<message*>& mes_v, std::vector<const message*>& c_mes_v): data(mes_v), c_data(c_mes_v)
    { }

    task::~task()
    { }

    void task::put(message* mes)
    { data.push_back(mes); }

    void task::put_c(const message* mes)
    { c_data.push_back(mes); }

    message& task::get_a(size_t id)
    { return *data[id]; }

    const message& task::get_c(size_t id)
    { return *c_data[id]; }

}
