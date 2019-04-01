#ifndef __MESSAGE_H__
#define __MESSAGE_H__

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
        data* const d;
    public:
        message(data* const _d = nullptr);
        virtual ~message();
        data* const get_data();
        virtual void send(int proc) = 0;
        virtual void recv(int proc) = 0;
    };

}

#endif // __MESSAGE_H__
