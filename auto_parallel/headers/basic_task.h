#ifndef __BASIC_TASK_H__
#define __BASIC_TASK_H__

#include "message.h"
#include <vector>

namespace auto_parallel
{

    class task
    {
    protected:
        std::vector<message*> data_v;
    public:
        task();
        task(message* dat);
        task(std::vector<message*>& mes_v);
        virtual ~task();
        virtual task* perform() = 0;
        friend class task_graph;
    };

}

#endif // __BASIC_TASK_H__
