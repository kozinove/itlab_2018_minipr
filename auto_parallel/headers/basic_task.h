#ifndef __BASIC_TASK_H__
#define __BASIC_TASK_H__

#include "message.h"

namespace auto_parallel
{

    class task
    {
    private:
        int id;
    public:
        task();
        virtual ~task();
        virtual task* perform(message* p) = 0;
    };

}

#endif // __BASIC_TASK_H__
