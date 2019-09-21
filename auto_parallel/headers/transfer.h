#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#include "mpi.h"
#include "message.h"

namespace auto_parallel
{

    class sender
    {
    private:

        MPI_Comm comm;
        int tag;
        int proc;
        message* mes;

    public:

        sender(MPI_Comm _comm = MPI_COMM_WORLD, int _proc = 0);
        void send(void* buf, int size, MPI_Datatype type);
        void isend(void* buf, int size, MPI_Datatype type);
        void big_send(void* buf, size_t size, MPI_Datatype type);
        void big_isend(void* buf, size_t size, MPI_Datatype type);

        void set_proc(int _proc);
        void set_message(message* _mes);

    };

    class receiver
    {
    private:

        MPI_Comm comm;
        int tag;
        int proc; 
        message* mes;

    public:

        receiver(MPI_Comm _comm = MPI_COMM_WORLD, int _proc = 0);
        void recv(void* buf, int size, MPI_Datatype type);
        void irecv(void* buf, int size, MPI_Datatype type);
        void big_recv(void* buf, size_t size, MPI_Datatype type);
        void big_irecv(void* buf, size_t size, MPI_Datatype type);

        void set_proc(int _proc);
        void set_message(message* _mes);

    };

}

#endif // __TRANSFER_H__
