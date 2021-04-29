#ifndef MIN_HEAP_CPP
#define MIN_HEAP_CPP

#include <netinet/in.h>
#include <ctime>
#include <iostream>

#define BUFFER_SIZE 64

using std::exception;

class heap_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer *timer;
};

class heap_timer
{
public:
    heap_timer(int delay)
    {
        expire = time(nullptr) + delay;
    }
    time_t expire;
    void (*cb_func)(client_data *);
    client_data *user_data;
};

class time_heap
{
public:
    time_heap(int cap) : cur_size(0), capacity(cap)
    {
        array = new heap_timer *[capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }
    }
    time_heap(heap_timer **init_array, int size, int capacity) : cur_size(size), capacity(capacity)
    {
        if (capacity < size)
        {
            throw std::exception();
        }
        array = new heap_timer *[capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }
        if (size != 0)
        {
            for (int i = 0; i < size; ++i)
            {
                array[i] = init_array[i];
            }
            for (int i = (cur_size - 1) / 2; i >= 0; --i)
            {
                percolate_down(i);
            }
        }
    }
    ~time_heap()
    {
        for (int i = 0; i < cur_size; ++i)
        {
            delete array[i];
        }
        delete[] array;
    }

    void add_timer(heap_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        if (cur_size >= capacity)
        {
            resize();
        }
        int hole = cur_size;
        ++cur_size;
        int parent = 0;
        for (; hole > 0; hole = parent)
        {
            parent = (hole -1) / 2; // get parent index
            if(array[parent]->expire <= timer->expire)
            {
                break;
            }
            array[hole] = array[parent];
        }
    }

private:
    void percolate_down(int hole)
    {
        heap_timer *temp = array[hole];
        int child = 0;
        for (; (2 * hole + 1) <= (cur_size - 1); hole = child)
        {
            child = 2 * hole + 1;
            if ((child < (cur_size - 1)) && (array[child + 1]->expire < array[child]->expire))
            {
                child += 1;
            }
            if (array[child]->expire < temp->expire)
            {
                array[hole] = array[child];
            }
            else
            {
                break;
            }
        }
        array[hole] = temp;
    }

    void resize()
    {
        heap_timer **temp = new heap_timer *[2 * capacity];
        for (int i = 0; i < 2 * capacity; ++i)
        {
            temp[i] = nullptr;
        }
        if (!temp)
        {
            throw std::exception();
        }
        capacity *= 2;
        for (int i = 0; i < cur_size; ++i)
        {
            temp[i] = array[i];
        }
        delete[] array;
        array = temp;
    }

    heap_timer **array;
    int capacity;
    int cur_size;
};
#endif /* MIN_HEAP_CPP */
