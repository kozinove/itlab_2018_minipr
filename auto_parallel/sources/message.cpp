#include "message.h"

namespace auto_parallel
{

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
