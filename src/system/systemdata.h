#ifndef SYSTEMDATA_H
#define SYSTEMDATA_H

#include <unordered_map>

class SystemLoader;
class SystemValidator;

namespace systemdata {
    class Mapping;

    enum SchedulerType {
        SCHEDTYPE_GLOBAL,
        SCHEDTYPE_PARTITIONED,
        SCHEDTYPE_HYBRID
    };

    enum Algorithm {
        SCHED_NULL,
        SCHED_STATIC,
        SCHED_EDF
    };

    enum TaskType {
        TASKTYPE_FIXED,
        TASKTYPE_MIGRATING
    };

    class Scheduler {
        friend class ::SystemValidator;
        std::string name;
        Algorithm algorithm;
        SchedulerType type;
        std::string mapping_name;
        Mapping *mapping;

        public:
        Scheduler(const std::string &name, Algorithm algorithm,
                  SchedulerType type, const std::string &mapping_name) :
                      name(name), algorithm(algorithm), type(type),
                      mapping_name(mapping_name) {}

        std::string get_name() const {
            return name;
        }

        Algorithm get_algorithm() const {
            return algorithm;
        }

        Mapping* get_mapping() const {
            return mapping;
        }

        SchedulerType get_type() const {
            return type;
        }
    };

    class Task {
        friend class ::SystemValidator;
        std::string name;
        int wcet;
        int read_delay;
        int write_delay;
        int start_time;
        int period;
        int deadline;
        int priority;
        TaskType type;

        public:
        Task(const std::string &name, int wcet, int read_delay,
             int write_delay, int start_time, int period, int deadline,
             int priority, TaskType type) :
                 name(name), wcet(wcet), read_delay(read_delay),
                 write_delay(write_delay), start_time(start_time),
                 period(period), deadline(deadline), priority(priority), 
                 type(type) {}

        std::string get_name() const {
            return name;
        }

        int get_wcet() const {
            return wcet;
        }

        int get_read_delay() const {
            return read_delay;
        }

        int get_write_delay() const {
            return write_delay;
        }

        int get_start_time() const {
            return start_time;
        }

        int get_period() const {
            return period;
        }

        int get_deadline() const {
            return deadline;
        }

        int get_priority() const {
            return priority;
        }
    };

    class Processor {
        friend class ::SystemLoader;
        friend class ::SystemValidator;
        std::string name;
        std::string scheduler_name;
        Scheduler *scheduler;

        public:
        Processor(const std::string &name, const std::string &scheduler_name) :
            name(name), scheduler_name(scheduler_name) {};

        std::string get_name() const {
            return name;
        }

        Scheduler *get_scheduler() const {
            return scheduler;
        }
    };

    class Mapping {
        friend class ::SystemLoader;
        friend class ::SystemValidator;
        class TaskEntry {
            friend class ::SystemValidator;
            std::string task_name;
            Task* task;

            public:
            TaskEntry(const std::string &task_name) : task_name(task_name) {};

            const Task* get_task() const {
                return task;
            }
        };

        class ProcessorEntry {
            friend class ::SystemLoader;
            friend class ::SystemValidator;
            std::string processor_name;
            Processor *processor;
            std::vector<TaskEntry*> tasks;

            public:
            ProcessorEntry(const std::string &processor_name) :
                processor_name(processor_name) {};

            ~ProcessorEntry() {
                for (auto it : tasks) {
                    delete it;
                }
            }

            const Processor* get_processor() const {
                return processor;
            }

            const std::vector<TaskEntry*>& get_task_entries() const {
                return tasks;
            }
        };

        std::string name;
        std::vector<ProcessorEntry*> entries;

        public:
        std::string get_name() const {
            return name;
        }

        const std::vector<ProcessorEntry*>& get_entries() const {
            return entries;
        }

        ~Mapping() {
            for (auto it : entries) {
                delete it;
            }
        }
    };

    class Fifo {
        std::string name;
        int size;

        public:
        Fifo(const std::string &name, int size) : name(name), size(size) {}

        std::string get_name() const {
            return name;
        }

        int get_size() const {
            return size;
        }
    };

    class System {
        friend class ::SystemLoader;
        friend class ::SystemValidator;
        std::unordered_map<std::string, Scheduler*> schedulers;
        std::unordered_map<std::string, Task*> tasks;
        std::unordered_map<std::string, Processor*> processors;
        std::unordered_map<std::string, Mapping*> mappings;
        std::unordered_map<std::string, Fifo*> fifos;

        public:
        bool addScheduler(Scheduler *scheduler) {
            if (!scheduler) return false;
            auto ret = schedulers.insert(std::make_pair(scheduler->get_name(), scheduler));
            return ret.second;
        }

        bool addTask(Task *task) {
            if (!task) return false;
            auto ret = tasks.insert(std::make_pair(task->get_name(), task));
            return ret.second;
        }

        bool addProcessor(Processor *processor) {
            if (!processor) return false;
            auto ret = processors.insert(std::make_pair(processor->get_name(), processor));
            return ret.second;
        }

        bool addMapping(Mapping *mapping) {
            if (!mapping) return false;
            auto ret = mappings.insert(std::make_pair(mapping->get_name(), mapping));
            return ret.second;
        }

        bool addFifo(Fifo *fifo) {
            if (!fifo) return false;
            auto ret = fifos.insert(std::make_pair(fifo->get_name(), fifo));
            return ret.second;
        }

        const std::unordered_map<std::string, Processor*>& get_processors() const {
            return processors;
        }

        const std::unordered_map<std::string, Scheduler*>& get_schedulers() const {
            return schedulers;
        }

        const std::unordered_map<std::string, Task*>& get_tasks() const {
            return tasks;
        }

        const std::unordered_map<std::string, Mapping*>& get_mappings() const {
            return mappings;
        }

        const Fifo* get_fifo(const std::string &fifo_name) const {
            auto iter = fifos.find(fifo_name);
            if (iter == fifos.end()) {
                return NULL;
            }

            return iter->second;
        }

        ~System() {
            for (auto it : schedulers) {
                delete it.second;
            }

            for (auto it : tasks) {
                delete it.second;
            }

            for (auto it : processors) {
                delete it.second;
            }

            for (auto it : mappings) {
                delete it.second;
            }

            for (auto it : fifos) {
                delete it.second;
            }
        }
    };
}

#endif // SYSTEMDATA_H
