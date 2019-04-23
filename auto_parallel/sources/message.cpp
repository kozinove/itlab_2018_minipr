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

    void message::simple_send(unsigned size, int proc)
    {
        if (d == nullptr)
            MPI_Send(d, 0, MPI_BYTE, proc, 0, MPI_COMM_WORLD);
        else
            MPI_Send(d, size, MPI_BYTE, proc, 0, MPI_COMM_WORLD);
    }

    void message::simple_recv(unsigned size, int proc)
    {
        MPI_Status status;
        int count;
        MPI_Probe(proc, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_BYTE, &count);
        if (count > 0)
        {
            if (d == nullptr)
                d = (data*)new char[size];
            MPI_Recv(d, size, MPI_BYTE, proc, 0, MPI_COMM_WORLD, &status);
        }
        else
        {
            if (d != nullptr)
            {
                delete d;
                d = nullptr;
            }
            MPI_Recv(d, 0, MPI_BYTE, proc, 0, MPI_COMM_WORLD, &status);
        }
    }

}
