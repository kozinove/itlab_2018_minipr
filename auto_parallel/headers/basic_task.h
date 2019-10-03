#ifndef __BASIC_TASK_H__
#define __BASIC_TASK_H__

#include "message.h"
#include <vector>
#include <functional>

namespace auto_parallel
{

    class task;

    class task_giver_base
    {
    public:

        task_giver_base();
        virtual ~task_giver_base();

        virtual task* get_task() = 0;

    };

    template<typename Type>
    class task_giver: public task_giver_base
    {
    private:

        static int my_id;

    public:

        task_giver();
        ~task_giver();

        task* get_task();
        static int get_id();

        friend class task_factory;
    };

    template<typename Type>
    int task_giver<Type>::my_id = -1;

    template<typename Type>
    task_giver<Type>::task_giver()
    { }

    template<typename Type>
    task_giver<Type>::~task_giver()
    { }

    template<typename Type>
    task* task_giver<Type>::get_task()
    {
        return new Type();
    }

    template<typename Type>
    int task_giver<Type>::get_id()
    {
        return my_id;
    }

    class task
    {
    private:

        std::vector<size_t> created;

    protected:

        std::vector<message*> data;
        std::vector<const message*> c_data;

        template<typename Task>
        void create_task(std::vector<message*>& mes_v, std::vector<const message*>& c_mes_v);

        template<typename Message>
        void create_message();

    public:

        task(std::vector<message*>& mes_v = std::vector<message*>::vector(), std::vector<const message*>& c_mes_v = std::vector<const message*>::vector());

        virtual ~task();
        virtual void perform() = 0;

        void put_a(message* mes);
        void put_c(const message* mes);

        message& get_a(size_t id);
        const message& get_c(size_t id);

        friend class task_graph;
        friend class parallelizer;
    };

    class task_factory
    {
    private:
        
        static std::vector<task_giver_base*> v;

    public:

        template<typename Type>
        static void add();

        //template<typename Type, typename ...Types>
        //static void add();

        static task* get(size_t id);
        
    };

    template<typename Type>
    void task_factory::add()
    {
        if (task_giver<Type>::get_id() > -1)
            return;
        task_giver<Type>::my_id = v.size();
        v.push_back(new task_giver<Type>);
    }

}

#endif // __BASIC_TASK_H__
