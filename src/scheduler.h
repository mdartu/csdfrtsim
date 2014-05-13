#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <systemc.h>

enum SchedulingParameter {
    PARAM_WCET,
    PARAM_START_TIME,
    PARAM_PERIOD,
    PARAM_DEADLINE,
    PARAM_PRIORITY
};

class sc_schedulable_module;

class TaskData {
};

class Task {
    friend class sc_scheduler;
    const sc_schedulable_module *module;
    sc_event run_event;
    TaskData *taskdata;

public:
    Task();
    Task(const sc_schedulable_module *module);
    ~Task();
    void set_module(const sc_schedulable_module *module);
    void set_taskdata(TaskData *taskdata);
    TaskData* get_taskdata() const;
    std::string get_name() const;
};

class Processor {
    friend class sc_scheduler;
    std::string name;
    Task *previous;
    Task *current;
    Task *next;
public:
    void set_next(Task *task);
    void set_name(const std::string &name);
    std::string get_name() const;
    Task* get_previous() const;
    Task* get_current() const;
    Task* get_next() const;
};

class TaskSet {
    std::vector<Task*> tasks;
    Processor *processor;
public:
    TaskSet(Processor *processor);
    void add_task(Task* task);
    const std::vector<Task*>& get_tasks() const;
    const Processor *get_processor() const;
};

class Scheduler {
    std::string name;
protected:
    std::vector<Task*> tasks;
    std::vector<Processor*> processors;
    std::vector<TaskSet*> tasksets;
public:
    void set_name(const std::string &name);
    std::string get_name() const;
    virtual void add_task(Task* task);
    virtual void add_processor(Processor *processor);
    virtual void add_taskset(TaskSet *taskset);
    virtual void set_parameter(Task *task, SchedulingParameter param, const void *value);
    virtual void init();
    virtual void run() = 0;
};

#endif // SCHEDULER_H
