#ifndef SC_SCHEDULER_H
#define SC_SCHEDULER_H

#include <vector>
#include <unordered_map>
#include <systemc.h>
#include "scheduler.h"

class sc_schedulable_module;
class Monitor;

class sc_scheduler : public sc_module {
private:
    std::unordered_map<const sc_schedulable_module*, Task*> task_table;
    std::vector<Task*> tasks;
    std::vector<TaskSet*> tasksets;
    std::vector<Processor*> processors;
    std::vector<Monitor*> monitors;
    std::vector<Scheduler*> schedulers;

    int counter;
    sc_mutex m;
    sc_event preempt_event;

    void run();

public:
    sc_in<bool> clk;

    typedef sc_scheduler SC_CURRENT_USER_MODULE;
    sc_scheduler(const sc_module_name &name);
    ~sc_scheduler();

    void add_scheduler(Scheduler *sched);
    void add_task(Task *task);
    void add_processor(Processor *processor);
    void add_monitor(Monitor *monitor);

    sc_event& run_event(const sc_schedulable_module* mod);
};

#endif // SC_SCHEDULER_H
