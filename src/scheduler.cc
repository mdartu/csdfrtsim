#include <algorithm>
#include <string>
#include "scheduler.h"
#include "sc_schedulable_module.h"

void Scheduler::set_name(const std::string &name) {
    this->name = name;
}

std::string Scheduler::get_name() const {
    return name;
}

void Scheduler::add_task(Task* task) {
    tasks.push_back(task);
}

void Scheduler::add_processor(Processor *processor) {
    processors.push_back(processor);
}

void Scheduler::add_taskset(TaskSet *taskset) {
    tasksets.push_back(taskset);
}

void Scheduler::set_parameter(Task *task, SchedulingParameter param, const void *value) {
}

void Scheduler::init() {
}

void Processor::set_next(Task *task) {
    next = task;
}

void Processor::set_name(const std::string &name) {
    this->name = name;
}

std::string Processor::get_name() const {
    return name;
}

Task* Processor::get_previous() const {
    return previous;
}

Task* Processor::get_current() const {
    return current;
}

Task* Processor::get_next() const {
    return next;
}

Task::Task() {
    module = NULL;
    taskdata = NULL;
}

Task::Task(const sc_schedulable_module *module) : module(module) {
    taskdata = NULL;
}

Task::~Task() {
    delete taskdata;
}

void Task::set_module(const sc_schedulable_module *module) {
    this->module = module;
}

void Task::set_taskdata(TaskData *taskdata) {
    this->taskdata = taskdata;
}

std::string Task::get_name() const {
    return module->name();
}

TaskData* Task::get_taskdata() const {
    return this->taskdata;
}

void TaskSet::add_task(Task* task) {
    this->tasks.push_back(task);
}

const std::vector<Task*>& TaskSet::get_tasks() const {
    return tasks;
}

const Processor *TaskSet::get_processor() const {
    return processor;
}
