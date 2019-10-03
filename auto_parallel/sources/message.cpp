#include "message.h"

namespace auto_parallel
{

    message_giver_base::message_giver_base()
    { }

    message_giver_base::~message_giver_base()
    { }


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

    std::vector<message_giver_base*> message_factory::v;

    message* message_factory::get(size_t id, message::init_info_base* info)
    { return v[id]->get_message(info); }

    message* message_factory::get_part(size_t id, message* p, message::part_info_base* info)
    { return v[id]->get_part_from(p, info); }

}
