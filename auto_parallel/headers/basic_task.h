#ifndef __BASIC_TASK_H__
#define __BASIC_TASK_H__

#include "message.h"
#include <vector>

namespace auto_parallel
{

    class task
    {
    protected:
        // вектора с данными
        std::vector<message*> data_v;
        std::vector<bool> mods; // здесь хранитс€ информаци€ о том изен€ютс€ л данные в задаче
    public:

        task();
        task(message* dat);
        task(message* dat, bool mode);
        task(std::vector<message*>& mes_v);
        task(std::vector<message*>& mes_v, std::vector<bool>& mode_v);

        virtual ~task();
        virtual void perform() = 0; // выполнение функции

        friend class task_graph;
        friend class parallelizer;
    };

}

#endif // __BASIC_TASK_H__
