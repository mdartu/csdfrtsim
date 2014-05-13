#include <iostream>
#include "sc_schedulable_module.h"
#include "sc_scheduler.h"
#include "scheduler.h"
#include "edfscheduler.h"
#include "globaledfscheduler.h"
#include "systembuilder.h"
#include "process.h"
#include "monitor.h"
using namespace std;

static int gcd(int a, int b) {
    int t;
    while (b != 0) {
        t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

SystemBuilder::SystemBuilder(systemdata::System *system, sc_scheduler *sc_sched) {
    this->system = system;
    this->sc_sched = sc_sched;
    this->max_start_time = -1;
    this->hyperperiod = 1;

    // Create tasks
    for (auto t : system->get_tasks()) {
        Task *task = new Task();
        task_map.insert(make_pair(t.second->get_name(), task));
    }

    // Create schedulers
    for (auto sched : system->get_schedulers()) {
        Scheduler *s;
        switch (sched.second->get_algorithm()) {
            case systemdata::SCHED_EDF:
                switch (sched.second->get_type()) {
                    case systemdata::SCHEDTYPE_PARTITIONED:
                        s = new EDFScheduler();
                        break;
                    case systemdata::SCHEDTYPE_GLOBAL:
                        s = new GlobalEDFScheduler();
                        break;
                    default:
                        cerr << "Fatal error: unsupported type for EDF scheduler '" << sched.second->get_name() << "'" << endl;
                        exit(1);
                }
                break;
            default:
                cerr << "Fatal error: unsupported scheduling algorithm for scheduler '" << sched.second->get_name() << "'" << endl;
                exit(1);
        }

        s->set_name(sched.second->get_name());
        scheduler_map.insert(std::make_pair(s->get_name(), s));
        this->sc_sched->add_scheduler(s);
    }

    // Create processors
    for (auto p : system->get_processors()) {
        Processor *proc = new Processor();
        proc->set_name(p.second->get_name());
        processor_map.insert(make_pair(p.second->get_name(), proc));
        this->sc_sched->add_processor(proc);

        // Find the scheduler for this processor, and register the processor with it
        auto s = scheduler_map.find(p.second->get_scheduler()->get_name());
        if (s == scheduler_map.end()) {
            cerr << "Scheduler for processor '" << p.second->get_name() << "' not found" << endl;
            exit(1);
        } else {
            (*s).second->add_processor(proc);
        }
    }

    // Create mappings (TODO)
    /*auto t = taskset_map.find(sched.second->get_mapping());
    cout << "Mapping name: " << sched.second->get_mapping()->get_name() << endl;
    if (t == taskset_map.end()) {
        // Create vector of task sets
        vector<TaskSet> *tasksets = new vector<TaskSet>(sched.second->get_mapping().get_entries().size());
        //for (auto it : sched.second->get_mapping().get_entries()) {
            //tasksets
        //}
    } else {
        cout << "Mapping found" << endl;
    }*/
}

int SystemBuilder::get_num_schedulers() const {
    return system->get_schedulers().size();
}

int SystemBuilder::get_fifo_size(const char *name, int def) const {
    const systemdata::Fifo *fifo = system->get_fifo(name);
    if (!fifo) {
        cerr << "Warning: no size defined for fifo '" << name << "', assuming default value (" << def << ")" << endl;
        return def;
    }
    return fifo->get_size();
}

void SystemBuilder::create_processors(std::vector<Processor*> &processors) {
    for (auto p : processor_map) {
        processors.push_back(p.second);
    }
}

void SystemBuilder::set_delays(const char *name, Process *process) {
    auto t = system->get_tasks().find(name);
    if (t == system->get_tasks().end()) {
        cerr << "No delays found for task '" << name << "'" << endl;
        exit(1);
    }
    process->set_delays(t->second->get_read_delay(), t->second->get_wcet(), t->second->get_write_delay());
}

void SystemBuilder::set_scheduling_parameters(Task *task, Scheduler *scheduler) const {
    auto t = system->get_tasks().find(task->get_name());

    if (t == system->get_tasks().end()) {
        cerr << "Information for task '" << task->get_name() << "' not found." << endl;
        exit(1);
    } else {
        // wcet is composed of task wcet + read delay + write delay
        int wcet = t->second->get_wcet() + t->second->get_read_delay() + t->second->get_write_delay();
        int start_time = t->second->get_start_time();
        int period = t->second->get_period();
        int deadline = t->second->get_deadline();
        int priority = t->second->get_priority();
        scheduler->set_parameter(task, PARAM_WCET, static_cast<const void*>(&wcet));
        scheduler->set_parameter(task, PARAM_START_TIME, static_cast<const void*>(&start_time));
        scheduler->set_parameter(task, PARAM_PERIOD, static_cast<const void*>(&period));
        scheduler->set_parameter(task, PARAM_DEADLINE, static_cast<const void*>(&deadline));
        scheduler->set_parameter(task, PARAM_PRIORITY, static_cast<const void*>(&priority));
    }
}

void SystemBuilder::set_monitoring_parameters(Monitor *monitor) const {
    for (auto task : task_map) {

        auto t = system->get_tasks().find(task.second->get_name());

        // wcet is composed of task wcet + read delay + write delay
        int wcet = t->second->get_wcet() + t->second->get_read_delay() + t->second->get_write_delay();
        int start_time = t->second->get_start_time();
        int period = t->second->get_period();
        int deadline = t->second->get_deadline();
        int priority = t->second->get_priority();

        monitor->set_parameter(task.second, PARAM_WCET, static_cast<const void*>(&wcet));
        monitor->set_parameter(task.second, PARAM_START_TIME, static_cast<const void*>(&start_time));
        monitor->set_parameter(task.second, PARAM_PERIOD, static_cast<const void*>(&period));
        monitor->set_parameter(task.second, PARAM_DEADLINE, static_cast<const void*>(&deadline));
        monitor->set_parameter(task.second, PARAM_PRIORITY, static_cast<const void*>(&priority));
    }
}

Scheduler* SystemBuilder::get_scheduler_for_task(const char *name) const {
    systemdata::Task *task;
    auto t = system->get_tasks().find(name);
    if (t == system->get_tasks().end()) {
        cerr << "Couldn't find task information for '" << name << "'" << endl;
        exit(1);
    }
    task = t->second;

    // Find the mapping in which the task appears
    systemdata::Mapping *mapping = NULL;
    for (auto m : system->get_mappings()) {
        for (auto pe : m.second->get_entries()) {
            for (auto te : pe->get_task_entries()) {
                if (te->get_task()->get_name() == name) {
                    mapping = m.second;
                    break;
                }
            }
        }
    }

    if (!mapping) {
        // SystemValidator should have checked this
        cerr << "Internal error: task '" << name << "' does not appear in a mapping" << endl;
        exit(1);
    }

    // Find scheduler that uses this mapping
    systemdata::Scheduler *scheduler = NULL;
    for (auto s : system->get_schedulers()) {
        if (s.second->get_mapping() == mapping) {
            scheduler = s.second;
            break;
        }
    }

    if (!scheduler) {
        cerr << "Mapping '" << mapping->get_name() << "' is not used by any scheduler" << endl;
        exit(1);
    }

    auto s = scheduler_map.find(scheduler->get_name());
    if (s == scheduler_map.end()) {
        // This should never happen
        cerr << "Internal error: no sc_scheduler object found for scheduler '" << scheduler->get_name() << "'" << endl;
        exit(1);
    }

    return s->second;
}

void SystemBuilder::create_task(sc_schedulable_module *sc_mod) {
    Task *task;
    auto t = task_map.find(sc_mod->name());
    if (t == task_map.end()) {
        cerr << "Couldn't find task information for process '" << sc_mod->name() << endl;
    }
    task = t->second;

    task->set_module(sc_mod);

    // Register the task with its scheduler
    // TODO: Multiple schedulers for task
    this->sc_sched->add_task(task);
    Scheduler *sched = get_scheduler_for_task(sc_mod->name());
    sched->add_task(task);

    this->set_scheduling_parameters(task, sched);

    // Update information for default simulation time
    auto ts = system->get_tasks().find(task->get_name());
    if (ts == system->get_tasks().end()) {
        cerr << "Information for task '" << task->get_name() << "' not found." << endl;
        exit(1);
    } else {
        int start_time = ts->second->get_start_time();
        int period = ts->second->get_period();
        if (start_time > this->max_start_time) {
            this->max_start_time = start_time;
        }
        this->hyperperiod = lcm(this->hyperperiod, period);
    }
}

int SystemBuilder::get_default_simulation_time() const {
    return this->max_start_time + 2*this->hyperperiod;
}
