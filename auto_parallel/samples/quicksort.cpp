#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <algorithm>
#include <iostream>
#include "parallel.h"
using namespace std;
using namespace auto_parallel;

class arrray: public message
{
public:

    int* p;
    size_t size;

    struct init_info: public init_info_base
    {
        size_t size;
        
        void send(const sender& se)
        { se.send(&size, 1, MPI_LONG_LONG); }

        void recv(const receiver& re)
        { re.recv(&size, 1, MPI_LONG_LONG); }
    };

    struct part_info: public part_info_base
    {
        int offset;
        size_t size;

        void send(const sender& se)
        {
            se.send(&offset, 1, MPI_INT);
            se.send(&size, 1, MPI_LONG_LONG);
        }

        void recv(const receiver& re)
        {
            re.recv(&offset, 1, MPI_INT);
            re.recv(&size, 1, MPI_LONG_LONG);
        }
    };

    arrray(init_info* ii): size(ii->size)
    { p = new int[ii->size]; }

    arrray(message* m, part_info* pi): size(pi->size)
    { p = ((arrray*)m)->p + pi->offset; }

    void send(const sender& se)
    { se.isend(p, size, MPI_INT); }

    void recv(const receiver& re)
    { re.irecv(p, size, MPI_INT); }
};

int main(int argc, char** argv)
{
    message_factory::add<arrray>();
    cout << message_creator<arrray>::get_id() << endl;
}