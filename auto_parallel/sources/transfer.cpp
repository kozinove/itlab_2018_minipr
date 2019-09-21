#include "transfer.h"

namespace auto_parallel
{

    sender::sender(MPI_Comm _comm, int _proc) : comm(_comm), proc(_proc)
    {
        mes = nullptr;
        tag = 0;
    }

}
