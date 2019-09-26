#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <queue>
#include "mpi.h"
#include "transfer.h"

namespace auto_parallel
{

    class message
    {
    private:

        std::queue<MPI_Request> req_q;

    public:

        message();
        virtual ~message();

        virtual void send(sender& se) = 0;
        virtual void recv(receiver& re) = 0;
        
        void wait_requests();

        friend class intracomm;

    };

}

#endif // __MESSAGE_H__
