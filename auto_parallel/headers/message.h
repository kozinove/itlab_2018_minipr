#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "mpi.h"

namespace auto_parallel
{

    struct data
    {
        data();
        virtual ~data() = 0;
    };

    class message
    {
    protected:
        // указатель на непосредственно данные. в начале работы рекомендуется оставлять nullptr
        data* d;
    public:
        const static bool read_only;
        const static bool read_write;
        // лучше всего оставлять nullptr но можно и инициализировать
        message(data* const _d = nullptr);
        virtual ~message();
        // возвращает константный указатель на данные
        data* const get_data();
        // способы пересылки данных
        virtual void send(int proc) = 0;
        virtual void recv(int proc) = 0;
    };

}

#endif // __MESSAGE_H__
