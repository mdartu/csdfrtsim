#include "globaledfscheduler.h"

bool GlobalEDFScheduler::greater_start_time::operator()(Task *x, Task *y) const {
    GlobalEDFSchedulerData *data_x = static_cast<GlobalEDFSchedulerData*>(x->get_taskdata());
    GlobalEDFSchedulerData *data_y = static_cast<GlobalEDFSchedulerData*>(y->get_taskdata());
    return data_x->release_time > data_y->release_time;
}

bool GlobalEDFScheduler::greater_deadline::operator()(Task *x, Task *y) const {
    GlobalEDFSchedulerData *data_x = static_cast<GlobalEDFSchedulerData*>(x->get_taskdata());
    GlobalEDFSchedulerData *data_y = static_cast<GlobalEDFSchedulerData*>(y->get_taskdata());
    return data_x->abs_deadline > data_y->abs_deadline;
}


GlobalEDFScheduler::GlobalEDFScheduler() {
    tick = 0;
}

void GlobalEDFScheduler::run() {
    // Move tasks from waiting to ready queue if their release time has passed
    while (!waiting_queue.empty()) {
        Task *t = waiting_queue.top();
        GlobalEDFSchedulerData *data = static_cast<GlobalEDFSchedulerData*>(t->get_taskdata());
        if (data->release_time <= tick) {
            waiting_queue.pop();
            ready_queue.push(t);
        } else {
            break;
        }
    }

    // Check if current task has finished
    for (auto processor : processors) {
        Task *running_task = processor->get_current();

        if (running_task) {
            GlobalEDFSchedulerData *running_data = static_cast<GlobalEDFSchedulerData*>(running_task->get_taskdata());
            running_data = static_cast<GlobalEDFSchedulerData*>(running_task->get_taskdata());
            if (running_data->ticks_remaining == 0) {
                running_data->release_time += running_data->period;
                running_data->abs_deadline = running_data->release_time + running_data->deadline;
                if (running_data->release_time == tick) {
                    ready_queue.push(running_task);
                } else {
                    waiting_queue.push(running_task);
                }
            }
        }
    }

    // Allocate tasks to processors
    for (auto processor : processors) {
        GlobalEDFSchedulerData *running_data = NULL;
        Task *running_task = processor->get_current();
        Task *next_task = running_task;

        if (running_task) {
            running_data = static_cast<GlobalEDFSchedulerData*>(running_task->get_taskdata());

            // Check for missed deadline
            if (running_data->ticks_remaining > 0 && running_data->abs_deadline <= tick) {
                cout << "[" << tick << ":"<< get_name() << "]: Task '" << running_task->get_name() << " missed its deadline (" << running_data->abs_deadline << ", " << running_data->ticks_remaining << " ticks remaining)" << endl;
            }
        }

        // Go IDLE if current task has finished
        if (running_task && running_data->ticks_remaining == 0) {
            next_task = NULL;
        }

        // Try to find a task to run if it has an earlier deadline
        if (!ready_queue.empty()) {
            GlobalEDFSchedulerData *top_data = static_cast<GlobalEDFSchedulerData*>(ready_queue.top()->get_taskdata());

            if ((running_task && top_data->abs_deadline < running_data->abs_deadline) || !running_task || running_task == ready_queue.top()) {
                next_task = ready_queue.top();
                ready_queue.pop();
                if (running_task && running_task != next_task && running_data->ticks_remaining > 0) {
                    // Re-queue the previous task, as it has not finished yet, but only if we don't immediately re-schedule
                    ready_queue.push(running_task);
                }
            }
        }

#if 0
        if (next_task != running_task) {
            if (running_task && next_task) {
                cout << "[" << tick << ":" << get_name() << "]: Switch from '" << running_task->get_name() << "' to '" << next_task->get_name() << "' on processor '" << processor->get_name() << "'" << endl;
            } else if (running_task) {
                cout << "[" << tick << ":" << get_name() << "]: Switch from '" << running_task->get_name() << "' to IDLE on processor '" << processor->get_name() << "'" << endl;
            } else if (next_task) {
                cout << "[" << tick << ":" << get_name() << "]: Switch from IDLE to '" << next_task->get_name() << "' on processor '" << processor->get_name() << "'" << endl;
            }
        }
#endif

        if (next_task) {
            GlobalEDFSchedulerData *next_data = static_cast<GlobalEDFSchedulerData*>(next_task->get_taskdata());
            if (next_data->ticks_remaining == 0) {
                next_data->ticks_remaining = next_data->wcet;
            }
        }

        processor->set_next(next_task);
        running_task = next_task;
        if (running_task) {
            running_data = static_cast<GlobalEDFSchedulerData*>(running_task->get_taskdata());
            running_data->ticks_remaining--;
        }
    }

    tick++;
}


void GlobalEDFScheduler::add_task(Task *task) {
    Scheduler::add_task(task);
    GlobalEDFSchedulerData *data = new GlobalEDFSchedulerData();
    task->set_taskdata(data);

    waiting_queue.push(task);
}

void GlobalEDFScheduler::set_parameter(Task *task, SchedulingParameter param, const void *value) {
    GlobalEDFSchedulerData *data = static_cast<GlobalEDFSchedulerData*>(task->get_taskdata());
    switch (param) {
        case PARAM_WCET:
            data->wcet = *static_cast<const int*>(value);
            break;
        case PARAM_START_TIME:
            data->start_time = *static_cast<const int*>(value);
            data->release_time = data->start_time;
            break;
        case PARAM_PERIOD:
            data->period = *static_cast<const int*>(value);
            break;
        case PARAM_DEADLINE:
            data->deadline = *static_cast<const int*>(value);
            data->abs_deadline = data->start_time + data->deadline;     // FIXME: Is start time guaranteed to be set at this point?
            break;
        default:
            break;
    }
}
