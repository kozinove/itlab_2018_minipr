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
        std::vector<bool> mods;
    public:
        const static bool read_only;
        const static bool read_write;

        task();
        task(message* dat);
        task(message* dat, bool mode);
        task(std::vector<message*>& mes_v);
        task(std::vector<message*>& mes_v, std::vector<bool>& mode_v);

        virtual ~task();
        virtual task* perform() = 0;

        friend class task_graph;
        friend class parallelizer;
    };

}

#endif // __BASIC_TASK_H__
