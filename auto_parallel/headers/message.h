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
        virtual void send() = 0;
        virtual void recv() = 0;
    };

}

#endif // __MESSAGE_H__
