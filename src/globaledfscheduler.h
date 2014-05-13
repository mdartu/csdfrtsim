#ifndef GLOBALEDFSCHEDULER_H
#define GLOBALEDFSCHEDULER_H

#include <queue>
#include <vector>
#include "scheduler.h"

class GlobalEDFSchedulerData : public TaskData {
    friend class GlobalEDFScheduler;
    int start_time;
    int wcet;
    int period;
    int deadline;
    int release_time;       // Absolute release time
    int abs_deadline;       // Absolute deadline
    int ticks_remaining;    // Ticks remaining for current period
};

class GlobalEDFScheduler : public Scheduler {
    struct greater_start_time {
        bool operator()(Task *x, Task *y) const;
    };

    struct greater_deadline {
        bool operator()(Task *x, Task *y) const;
    };

    std::priority_queue<Task*, std::vector<Task*>, greater_start_time> waiting_queue;
    std::priority_queue<Task*, std::vector<Task*>, greater_deadline> ready_queue;
    int tick;
public:
    GlobalEDFScheduler();
    void run();
    void add_task(Task *task);
    void set_parameter(Task *task, SchedulingParameter param, const void *value);
};

#endif // GLOBALEDFSCHEDULER_H

