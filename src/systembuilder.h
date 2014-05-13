#ifndef SYSTEMBUILDER_H
#define SYSTEMBUILDER_H

#include "system/systemdata.h"
#include <vector>
#include <unordered_map>

class Process;
class Processor;
class Scheduler;
class sc_scheduler;
class sc_schedulable_module;
class Task;
class Monitor;

class SystemBuilder {
    sc_scheduler *sc_sched;
    systemdata::System *system;
    std::unordered_map<std::string, Task*> task_map;
    std::unordered_map<std::string, Processor*> processor_map;
    std::unordered_map<std::string, Scheduler*> scheduler_map;
    //std::unordered_map<systemdata::Mapping*, std::vector<TaskSet>*> taskset_map;
    int max_start_time;
    int hyperperiod;
public:
    SystemBuilder(systemdata::System *system, sc_scheduler *sc_sched);
    int get_fifo_size(const char *name, int def) const;
    int get_num_schedulers() const;
    void create_processors(std::vector<Processor*> &processors);
    void set_delays(const char *name, Process *process);
    void set_scheduling_parameters(Task *task, Scheduler *scheduler) const;
    void set_monitoring_parameters(Monitor *monitor) const;
    Scheduler* get_scheduler_for_task(const char *name) const;
    void create_task(sc_schedulable_module *sc_mod);
    int get_default_simulation_time() const;
};

#endif // SYSTEMBUILDER_H
