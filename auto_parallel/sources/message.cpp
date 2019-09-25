#include "message.h"

namespace auto_parallel
{

    const bool message::read_only = true;
    const bool message::read_write = false;

    message::message()
    { }

    message::~message()
    { }

    void message::wait_requests()
    {
        while (req_q.size())
        {
            MPI_Wait(&req_q.front(), MPI_STATUS_IGNORE);
            req_q.pop();
        }
    }

}
