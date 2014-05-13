#ifndef MONITOR_H
#define MONITOR_H

#include "scheduler.h"

class Processor;
class Task;

class Monitor {
    int time;
protected:
    int get_time() const;
public:
    Monitor();
    virtual ~Monitor();
    void advance_time();
    virtual void add_processor(const Processor *p) = 0;
    virtual void add_task(const Task *t) = 0;
    virtual void task_preempted(const Task *t, const Processor *p) = 0;
    virtual void task_resumed(const Task *t, const Processor *p) = 0;
    virtual void simulation_finished() = 0;
    virtual void set_parameter(const Task *task, SchedulingParameter param, const void *value);
};

#endif // MONITOR_H
