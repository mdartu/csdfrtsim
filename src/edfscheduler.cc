#include "edfscheduler.h"

bool EDFScheduler::greater_start_time::operator()(Task *x, Task *y) const {
    EDFSchedulerData *data_x = static_cast<EDFSchedulerData*>(x->get_taskdata());
    EDFSchedulerData *data_y = static_cast<EDFSchedulerData*>(y->get_taskdata());
    return data_x->release_time > data_y->release_time;
}

bool EDFScheduler::greater_deadline::operator()(Task *x, Task *y) const {
    EDFSchedulerData *data_x = static_cast<EDFSchedulerData*>(x->get_taskdata());
    EDFSchedulerData *data_y = static_cast<EDFSchedulerData*>(y->get_taskdata());
    return data_x->abs_deadline > data_y->abs_deadline;
}


EDFScheduler::EDFScheduler() {
    tick = 0;
    running_task = NULL;
}

void EDFScheduler::run() {
    // Move tasks from waiting to ready queue if their release time has passed
    while (!waiting_queue.empty()) {
        Task *t = waiting_queue.top();
        EDFSchedulerData *data = static_cast<EDFSchedulerData*>(t->get_taskdata());
        if (data->release_time <= tick) {
            waiting_queue.pop();
            ready_queue.push(t);
        } else {
            break;
        }
    }

    EDFSchedulerData *running_data = NULL;
    Task *next_task = running_task;

    if (running_task) {
        running_data = static_cast<EDFSchedulerData*>(running_task->get_taskdata());
    }

    // If the ready queue contains a task with an earlier deadline, force a swtch
    if (!ready_queue.empty()) {
        // New arrival with earlier deadline, switch
        EDFSchedulerData *top_data = static_cast<EDFSchedulerData*>(ready_queue.top()->get_taskdata());
        if (running_task && top_data->abs_deadline < running_data->abs_deadline) {
            next_task = ready_queue.top();
            ready_queue.pop();
        }
    }

    // Try to switch tasks if the current task finished, or if the cpu is IDLE
    if (next_task == running_task && ((running_task && running_data->ticks_remaining == 0) || !running_task)) {
        // Task done, switch
        if (!ready_queue.empty()) {
            next_task = ready_queue.top();
            ready_queue.pop();
        } else {
            // IDLE
            next_task = NULL;
        }
    }

    // Check for missed deadline
    if (!no_deadline_missed_warning && running_task) {
        if (running_data->ticks_remaining > 0 && running_data->abs_deadline <= tick) {
            cout << "[" << tick << ":"<< get_name() << "]: Task '" << running_task->get_name() << " missed its deadline (" << running_data->abs_deadline << ", " << running_data->ticks_remaining << " ticks remaining)" << endl;
            no_deadline_missed_warning = true;
        }
    }

    // Perform the actual task switch
    if (next_task != running_task) {
        if (running_task && running_data->ticks_remaining == 0) {
            // Compute next release time
            running_data->release_time += running_data->period;
            running_data->abs_deadline = running_data->release_time + running_data->deadline;
            if (running_data->release_time == tick) {       // Can resume immediately
                next_task = running_task;
            } else {
                waiting_queue.push(running_task);
            }
        } else if (running_task) {
            // Task has not finished yet, return to ready queue
            ready_queue.push(running_task);
        }

        if (next_task) {
            EDFSchedulerData *next_data = static_cast<EDFSchedulerData*>(next_task->get_taskdata());
            if (next_data->ticks_remaining == 0) {
                next_data->ticks_remaining = next_data->wcet;      // -1, since we already run in this tick without decrementing
            }
        }

#if 0
        if (running_task && next_task) {
            cout << "[" << tick << ":" << get_name() << "]: Switch from '" << running_task->get_name() << "' to '" << next_task->get_name() << "'" << endl;
        } else if (running_task) {
            cout << "[" << tick << ":" << get_name() << "]: Switch from '" << running_task->get_name() << "' to IDLE" << endl;
        } else if (next_task) {
            cout << "[" << tick << ":" << get_name() << "]: Switch from IDLE to '" << next_task->get_name() << "'" << endl;
        }
#endif

        no_deadline_missed_warning = false;
    }
    running_task = next_task;
    if (running_task) {
        running_data = static_cast<EDFSchedulerData*>(running_task->get_taskdata());
    }

    processors[0]->set_next(running_task);
    if (running_task) {
        running_data->ticks_remaining--;
    }
    tick++;
}


void EDFScheduler::add_task(Task *task) {
    Scheduler::add_task(task);
    EDFSchedulerData *data = new EDFSchedulerData();
    task->set_taskdata(data);

    waiting_queue.push(task);
}

void EDFScheduler::set_parameter(Task *task, SchedulingParameter param, const void *value) {
    EDFSchedulerData *data = static_cast<EDFSchedulerData*>(task->get_taskdata());
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
            data->abs_deadline = data->start_time + data->deadline;
            break;
        default:
            break;
    }
}
