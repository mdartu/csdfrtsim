#ifndef ROUNDROBIN_H
#define ROUNDROBIN_H

#include "scheduler.h"

class RoundRobin;

class RoundRobinData : public TaskData {
    friend class RoundRobin;
    int ticks;
};

class RoundRobin : public Scheduler {
    static const int num_ticks = 2;
public:
    RoundRobin();
    void run();
    void add_task(Task *task);
};

#endif // ROUNDROBIN_H
