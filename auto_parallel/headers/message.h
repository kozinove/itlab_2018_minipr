#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <queue>
#include "mpi.h"
#include "transfer.h"

namespace auto_parallel
{

    struct data
    {
        data();
        virtual ~data() = 0;
    };

    class message
    {
    private:

        std::queue<MPI_Request> req_q;
        void wait_requests();

    protected:

        data* d;

    public:

        const static bool read_only;
        const static bool read_write;

        message(data* const _d = nullptr);
        virtual ~message();

        data* const get_data();

        virtual void send(sender se) = 0;
        virtual void recv(receiver re) = 0;

        friend class intracomm;

    };

}

#endif // __MESSAGE_H__
