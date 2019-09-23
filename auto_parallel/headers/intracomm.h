#ifndef __INTRACOMM_H__
#define __INTRACOMM_H__

#include "mpi.h"
#include "communicator.h"
#include "transfer.h"
#include "message.h"

namespace auto_parallel
{

    class intracomm: public communicator
    {

    public:

        intracomm(MPI_Comm _comm = MPI_COMM_WORLD);
        explicit intracomm(const intracomm& c);
        intracomm(const intracomm& c, int color, int key);
        ~intracomm();

        void send(message* mes, int proc);
        void recv(message* mes, int proc);
        void bcast(message* mes, int proc);

    };

}

#endif // __INTRACOMM_H__
