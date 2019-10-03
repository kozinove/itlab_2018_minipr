#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <queue>
#include <vector>
#include "mpi.h"
#include "transfer.h"

namespace auto_parallel
{

    class message
    {
    private:

        std::queue<MPI_Request> req_q;

    public:

        struct init_info_base
        {
            virtual void send(sender& se) = 0;
            virtual void recv(receiver& re) = 0;
        };

        struct part_info_base
        { 
            virtual void send(sender& se) = 0;
            virtual void recv(receiver& re) = 0;
        };

        message();
        virtual ~message();

        virtual void send(sender& se) = 0;
        virtual void recv(receiver& re) = 0;
        
        void wait_requests();

        friend class intracomm;

    };

    class message_giver_base
    {
    public:

        message_giver_base();
        virtual ~message_giver_base();

        virtual message* get_message(message::init_info_base* info) = 0;
        virtual message* get_part_from(message* p, message::part_info_base* info) = 0;
    };

    template<typename Type>
    class message_giver: public message_giver_base
    {
    private:

        static int my_id;

    public:

        message_giver();
        ~message_giver();

        message* get_message(message::init_info_base* info);
        message* get_part_from(message* p, message::part_info_base* info);

        static int get_id();

        friend class message_factory;
    };

    template<typename Type>
    int message_giver<Type>::my_id = -1;

    template<typename Type>
    message_giver<Type>::message_giver()
    { }

    template<typename Type>
    message_giver<Type>::~message_giver()
    { }

    template<typename Type>
    message* message_giver<Type>::get_message(message::init_info_base* info)
    {
        Type::init_info* p = (Type::init_info*)info;
        return new Type(p);
    }

    template<typename Type>
    message* message_giver<Type>::get_part_from(message* p, message::part_info_base* info)
    {
        Type::part_info* q = (Type::part_info*)info;
        return new Type(p, q);
    }

    template<typename Type>
    int message_giver<Type>::get_id()
    { return my_id }

    class message_factory
    {
    private:
        
        static std::vector<message_giver_base*> v;

    public:
        
        template<typename Type>
        static void add();

        static message* get(size_t id, message::init_info_base* info);
        static message* get_part(size_t id, message* p, message::part_info_base* info);
    };

    template<typename Type>
    void message_factory::add()
    {
        if (message_giver<Type>::my_id >= 0)
            return;
        message_giver<Type>::my_id = v.size();
        v.push_back(new message_giver<Type>());
    }

}

#endif // __MESSAGE_H__
