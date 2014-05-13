#ifndef STATSMONITOR_H
#define STATSMONITOR_H

#include <iostream>
#include <map>
#include "monitor.h"

struct ProcStats {
    int util;
    ProcStats() : util(0) {}
};

struct TaskStats {
    const Processor *proc;
    int et;
    int last_preempt;
    int last_resume;
    int migrations;

    TaskStats() : proc(0), et(0), last_preempt(0), last_resume(0), 
                  migrations(-1) {}
};

class StatsMonitor : public Monitor {
    std::map<const Processor*, ProcStats> proc_stats;
    std::map<const Task*, TaskStats> task_stats;
public:
    void add_processor(const Processor *p);
    void add_task(const Task *t);
    void task_preempted(const Task *t, const Processor *p);
    void task_resumed(const Task *t, const Processor *p);
    void simulation_finished();
    void write_stats(std::ostream &stream);
    ~StatsMonitor();
};

#endif // STATSMONITOR_H
