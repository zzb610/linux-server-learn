#ifndef TIME_WHEEL_TIMER_H
#define TIME_WHEEL_TIMER_H

#include <netinet/in.h>
#include <cstdio>

#define BUFFER_SIZE 64
class tw_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

class tw_timer
{
public:
    tw_timer(int rot, int ts) : next(nullptr), prev(nullptr), rotation(rot), time_slot(ts) {}
    int rotation;                   // how many rotations later timer works
    int time_slot;                  // which slot this timer in the timer wheel
    void (*cb_func)(client_data *); // call back function
    client_data *user_data;

    tw_timer *next;
    tw_timer *prev;
};

class time_wheel
{
public:
    time_wheel() : cur_slot(0)
    {
        for (int i = 0; i < N; ++i)
        {
            slots[i] = nullptr;
        }
    }
    ~time_wheel()
    {
        for (int i = 0; i < N; ++i)
        {
            tw_timer *tmp = slots[i];
            while (tmp)
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }
    tw_timer *add_timer(int timeout)
    {
        if (timeout < 0)
        {
            return nullptr;
        }
        int ticks = 0;
        if (timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI;
        }
        int rotation = ticks / N;
        // insert location
        int ts = (cur_slot + (ticks % N)) % N;
        tw_timer *timer = new tw_timer(rotation, ts);

        if (!slots[ts])
        {
            printf("add timer, rotation is %d, ts is %d , cur slot is %d", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    void del_timer(tw_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        int ts = timer->time_slot;
        if (timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if (slots[ts])
            {
                slots[ts]->prev = nullptr;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if (timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }
    void tick()
    {
        tw_timer *tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while (tmp)
        {
            printf("tick the timer once\n");
            if (tmp->rotation > 0) // not at time
            {
                --tmp->rotation;
                tmp = tmp->next;
            }
            else // at time
            {
                tmp->cb_func(tmp->user_data);
                if (tmp == slots[cur_slot]) // remove header
                {
                    slots[cur_slot] = tmp->next;
                    if (slots[cur_slot])
                    {
                        slots[cur_slot]->prev = nullptr;
                    }
                    delete tmp;
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if (tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer *tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = (cur_slot + 1) % N;
    }

private:
    static const int N = 60; // slot number
    static const int SI = 1; // slot interval

    tw_timer *slots[N]; // slots unordered
    int cur_slot;       // current slot
};

#endif /* TIME_WHEEL_TIMER_H */
