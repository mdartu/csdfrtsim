#include <iostream>
#include <string>
#include "systemvalidator.h"
using namespace std;

SystemValidator::SystemValidator(systemdata::System *system) {
    // Write errors to cerr
    errors = &cerr;
    this->system = system;
}

bool SystemValidator::validateSystem(systemdata::System *system) {
    // TODO: Check the system as a whole (e.g. how many schedulers, mixing schedulers of different types, ...)

    return true;
}

bool SystemValidator::validateScheduler(systemdata::Scheduler *scheduler) {
    if (!validateName(scheduler->name)) {
        *errors << "Invalid scheduler name '" << scheduler->name << "'" << endl;
        return false;
    }

    // Find mapping
    auto iter = system->mappings.find(scheduler->mapping_name);
    if (iter == system->mappings.end()) {
        *errors << "Mapping '" << scheduler->mapping_name << "' for scheduler '" << scheduler->name << "' not found" << endl;
        return false;
    }

    scheduler->mapping = iter->second;

    return true;
}

bool SystemValidator::validateTask(systemdata::Task *task) {
    bool ret = true;
    if (!validateName(task->name)) {
        *errors << "Invalid task name '" << task->name << "'" << endl;
        ret = false;
    }

    if (task->wcet <= 0) {
        *errors << "wcet for task '" << task->name << "' should be > 0" << endl;
        ret = false;
    }

    if (task->read_delay < 0) {
        *errors << "Read delay for task '" << task->name << "' should be >= 0" << endl;
        ret = false;
    }

    if (task->write_delay < 0) {
        *errors << "Write delay for task '" << task->name << "' should be >= 0" << endl;
        ret = false;
    }

    if (task->start_time < 0) {
        *errors << "start time for task '" << task->name << "' should be >= 0" << endl;
        ret = false;
    }

    if (task->period <= 0) {
        *errors << "period for task '" << task->name << "' should be > 0" << endl;
        ret = false;
    }

    if (task->deadline <= 0) {
        *errors << "deadline for task '" << task->name << "' should be > 0" << endl;
        ret = false;
    }

    if (task->priority <= 0) {
        *errors << "priority for task '" << task->name << "' should be > 0" << endl;
        ret = false;
    }

    if (task->wcet > task->period) {
        *errors << "wcet for task '" << task->name << "' should be <= period" << endl;
        ret = false;
    }

    if (task->deadline > task->period) {
        *errors << "deadline for task '" << task->name << "' should be <= period" << endl;
        ret = false;
    }

    if (task->wcet > task->deadline) {
        *errors << "wcet for task '" << task->name << "' should be <= deadline" << endl;
        ret = false;
    }

    return ret;
}

bool SystemValidator::validateProcessor(systemdata::Processor *processor) {
    if (!validateName(processor->name)) {
        *errors << "Invalid processor name '" << processor->name << "'" << endl;
        return false;
    }

    // Associate scheduler with processor
    auto iter = system->schedulers.find(processor->scheduler_name);
    if (iter == system->schedulers.end()) {
        *errors << "Scheduler '" << processor->scheduler_name << "' for processor '" << processor->name << "' not found" << endl;
        return false;
    }

    processor->scheduler = iter->second;

    return true;
}

bool SystemValidator::validateMapping(systemdata::Mapping *mapping) {
    bool ret = true;

    if (!validateName(mapping->name)) {
        *errors << "Invalid mapping name '" << mapping->name << "'" << endl;
        ret = false;
    }

    for (auto p : mapping->entries) {
        // Find processor
        auto proc = system->processors.find(p->processor_name);
        if (proc == system->processors.end()) {
            *errors << "Processor '" << p->processor_name << "' for mapping '" << mapping->name << "' not found" << endl;
            ret = false;
        } else {
            p->processor = proc->second;
        }

        for (auto t : p->tasks) {
            auto task = system->tasks.find(t->task_name);
            if (task == system->tasks.end()) {
                *errors << "Task '" << t->task_name << "' for mapping '" << mapping->name << "' not found" << endl;
                ret = false;
            } else {
                t->task = task->second;
            }
        }
    }

    return ret;
}

bool SystemValidator::validateName(const string &name) {
    for (char c : name) {
        // Only allow 'normal' ascii characters (no spaces or special characters)
        if (c < '!' || c > '~') {
            return false;
        }
    }

    return true;
}

bool SystemValidator::validate() {
    // Validate all schedulers, tasks, processors and mappings
    for (auto scheduler : system->schedulers) {
        if (!validateScheduler(scheduler.second)) return false;
    }

    for (auto task : system->tasks) {
        if (!validateTask(task.second)) return false;
    }

    for (auto processor : system->processors) {
        if (!validateProcessor(processor.second)) return false;
    }

    for (auto mapping : system->mappings) {
        if (!validateMapping(mapping.second)) return false;
    }

    // TODO

    if (!validateSystem(system)) {
        return false;
    }

    return true;
}
