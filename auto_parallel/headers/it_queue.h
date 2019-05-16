#ifndef __IT_QUEUE__
#define __IT_QUEUE__

#include <iterator>
#include <algorithm>

namespace auto_parallel
{

//    template<typename type>
//    class cycle_iterator: public std::iterator<std::input_iterator_tag, type>
//    {
//        private:
//
//
//
//        friend class it_queue;
//    };

    template<typename type>
    class it_queue
    {
        private:

        static size_t max_size;
        type* _value;
        size_t first, last, _size, _capasity;
        bool full;

        public:

        it_queue();
        ~it_queue();

        const type& front();
        void push(const type& val);
        void pop();

        const type& operator[](size_t n) const;

        bool empty();

        size_t size();
        size_t capasity();

        //cycle_iterator<type> begin();
        //cycle_iterator<type> end();
        //cycle_iterator<const type> begin() const;
        //cycle_iterator<const type> end() const;

    };

    template<typename type>
    size_t it_queue<type>::max_size = 1073741823u;

    template<typename type>
    it_queue<type>::it_queue()
    {
        first = last = 0u;
        full = true;
        _size = _capasity = 0u;
        _value = nullptr;
    }

    template<typename type>
    it_queue<type>::~it_queue()
    {

    }

    template<typename type>
    const type& it_queue<type>::front()
    {
        return _value[first];
    }

    template<typename type>
    void it_queue<type>::push(const type& val)
    {
        if (full)
        {
            if (_value == nullptr)
            {
                _value = new type[1u];
                _capasity = 1u;
            }
            else
            {
                type* p = new type[std::min((_capasity << 1),max_size)];
                for (size_t i = 0u, j = first; i < _capasity; ++i,j = (j + 1u) % _capasity)
                    p[i] = _value[j];
                first = 0u;
                last = _capasity;
                _capasity = std::min((_capasity << 1),max_size);
                delete[] _value;
                _value = p;
                full = false;
            }
        }
        _value[last] = val;
        last = (last + 1u) % _capasity;
        ++_size;
        if (first == last)
            full = true;
    }

    template<typename type>
    void it_queue<type>::pop()
    {
        (_value + first)->~type();
        first = (first + 1u) % _capasity;
        --_size;
        full = false;
    }

    template<typename type>
    const type& it_queue<type>::operator[](size_t n) const
    {
        return _value[(first + n) % _capasity];
    }

    template<typename type>
    bool it_queue<type>::empty()
    {
        return (first == last) && (!full);
    }

    template<typename type>
    size_t it_queue<type>::size()
    {
        return _size;
    }

    template<typename type>
    size_t it_queue<type>::capasity()
    {
        return _capasity;
    }

}
#endif // __IT_QUEUE__
