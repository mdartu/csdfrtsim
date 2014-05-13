#ifndef GRASPMONITOR_H
#define GRASPMONITOR_H

#include <fstream>
#include <string>
#include <map>
#include "monitor.h"

class GraspMonitor : public Monitor {
    struct TaskInfo {
        int start_time;
        int deadline;
        int period;
        int wcet;
        int current_period;
        int last_resume;
        int last_preempt;
        int et_current_period;
        int color;
        const Processor *processor;

    public:
        TaskInfo() : start_time(0), deadline(0), period(0),
                     current_period(-1), last_resume(-1),
                     last_preempt(-1), et_current_period(0),
                     color(0), processor(NULL) {}
    };

    std::map<const Task*, TaskInfo> task_data;
    std::ofstream output;
    int last_color;
public:
    GraspMonitor(const std::string &filename);
    ~GraspMonitor();
    void add_processor(const Processor *p);
    void add_task(const Task *t);
    void task_preempted(const Task *t, const Processor *p);
    void task_resumed(const Task *t, const Processor *p);
    void simulation_finished();
    void set_parameter(const Task *task, SchedulingParameter param, const void *value);
};

#endif // GRASPMONITOR_H
