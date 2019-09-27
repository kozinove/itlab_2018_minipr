#ifndef __BASIC_TASK_H__
#define __BASIC_TASK_H__

#include "message.h"
#include <vector>
#include <functional>

namespace auto_parallel
{

    class task
    {
    protected:

        std::vector<message*> data;
        std::vector<const message*> c_data;

    public:

        task(std::vector<message*>& mes_v = std::vector<message*>::vector(), std::vector<const message*>& c_mes_v = std::vector<const message*>::vector());

        virtual ~task();
        virtual void perform() = 0;

        void put(message* mes);
        void put_c(const message* mes);

        message& get_a(size_t id);
        const message& get_c(size_t id);

        friend class task_graph;
        friend class parallelizer;
    };

}

#endif // __BASIC_TASK_H__
