#include "parallelizer.h"

namespace auto_parallel
{

    parallelizer::parallelizer(int mode, task_graph& _tg, int* argc, char*** argv): tg(_tg)
    {
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    }

    parallelizer::~parallelizer()
    {
        MPI_Finalize();
    }

}
