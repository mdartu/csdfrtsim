#include "roundrobin.h"

RoundRobin::RoundRobin() {
}

void RoundRobin::run() {
    // Round-robin scheduler for a task list without priorities on multiple processors

    int p = 0;
    for (auto processor : processors) {
        // For each processor, pick the first task that is allowed and still has ticks left
        Task *run_task = NULL;
        for (auto taskset : tasksets) {
            if (taskset->get_processor() == processor) {
                for (auto task : taskset->get_tasks()) {
                    if (static_cast<RoundRobinData*>(task->get_taskdata())->ticks > 0) {
                        run_task = task;
                        break;
                    }
                }
            }
        }

        if (!run_task) {
            for (auto taskset : tasksets) {
                if (taskset->get_processor() == processor) {
                    for (auto task : taskset->get_tasks()) {
                        if (!run_task) run_task = task;
                        static_cast<RoundRobinData*>(task->get_taskdata())->ticks = num_ticks;
                    }
                }
            }
        }

        cout << "Running task: " << run_task->get_name() << " on processor " << p++ << endl;
        static_cast<RoundRobinData*>(run_task->get_taskdata())->ticks--;
        processor->set_next(run_task);
    }
}

void RoundRobin::add_task(Task *task) {
    Scheduler::add_task(task);
    RoundRobinData *data = new RoundRobinData();
    task->set_taskdata(data);
}
