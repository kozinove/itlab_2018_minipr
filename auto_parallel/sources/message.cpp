#include "message.h"

namespace auto_parallel
{

    const bool message::read_only = true;
    const bool message::read_write = false;

    data::data()
    {

    }

    data::~data()
    {

    }

    message::message(data* const _d): d(_d)
    {

    }

    message::~message()
    {

    }

    data* const message::get_data()
    {
        return d;
    }

}
