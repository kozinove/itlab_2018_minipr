#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "mpi.h"

namespace auto_parallel
{

    struct data
    {
        data();
        virtual ~data() = 0;
    };

    class message
    {
    protected:
        // ��������� �� ��������������� ������. � ������ ������ ������������� ��������� nullptr
        data* d;
    public:
        const static bool read_only;
        const static bool read_write;
        // ����� ����� ��������� nullptr �� ����� � ����������������
        message(data* const _d = nullptr);
        virtual ~message();
        // ���������� ����������� ��������� �� ������
        data* const get_data();
        // ������� ��������� ������
        virtual void send(int proc) = 0;
        virtual void recv(int proc) = 0;
    };

}

#endif // __MESSAGE_H__
