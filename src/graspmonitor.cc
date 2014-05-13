#include "scheduler.h"
#include "graspmonitor.h"
#include <iostream>
using namespace std;

const char *colors[] = { "#666666", "#EEEEEE", "#333333", "#AAAAAA" };

GraspMonitor::GraspMonitor(const string &filename) {
    last_color = 0;
    output.open(filename);
}

GraspMonitor::~GraspMonitor() {
    output.close();
}

void GraspMonitor::add_processor(const Processor *p) {
    output << "newProcessor " << p << " -name \"" << p->get_name() << "\"" << endl;
}

void GraspMonitor::add_task(const Task *t) {
    output << "newTask " << t << " -name \"" << t->get_name() << "\" -color " << colors[last_color] << endl;
    last_color = (last_color + 1) % (sizeof(colors) / sizeof(colors[0]));
}

void GraspMonitor::task_preempted(const Task *t, const Processor *p) {
    task_data[t].et_current_period += get_time() - task_data[t].last_resume;

    output << "plot " << get_time() << " jobPreempted " << t << ".0" << endl;

    if (task_data[t].et_current_period >= task_data[t].wcet) {
        output << "plot " << get_time() << " jobCompleted " << t << ".0" << endl;
    }

    task_data[t].last_preempt = get_time();
}

void GraspMonitor::task_resumed(const Task *t, const Processor *p) {
    int period_no =  (get_time() - task_data[t].start_time) / task_data[t].period;
    if (period_no != task_data[t].current_period) {
        // If we have not observed the task in its current period before, it released a new job
        int start_time = period_no * task_data[t].period + task_data[t].start_time;
        task_data[t].current_period = period_no;
        task_data[t].et_current_period = 0;
        output << "plot " << start_time << " jobArrived " << t << ".0" << " " << t << " -processor " << p << endl;
    }

    output << "plot " << get_time() << " jobResumed " << t << ".0" << " -processor " << p << endl;

    task_data[t].last_resume = get_time();
    task_data[t].processor = p;
}

void GraspMonitor::simulation_finished() {
    // Finish tasks that were still running at the end of the simulation
    for (auto &t : task_data) {
        if (t.second.last_resume >= t.second.last_preempt && t.second.last_resume > 0) {      // we need to check >=, because a task might be preempted and resumed in the same cycle (migration)
            task_preempted(t.first, t.second.processor);
        }
    }
}

void GraspMonitor::set_parameter(const Task *task, SchedulingParameter param, const void *value) {
    switch (param) {
        case PARAM_WCET:
            task_data[task].wcet = *static_cast<const int*>(value);
            break;
        case PARAM_DEADLINE:
            task_data[task].deadline = *static_cast<const int*>(value);
            break;
        case PARAM_PERIOD:
            task_data[task].period = *static_cast<const int*>(value);
            break;
        case PARAM_START_TIME:
            task_data[task].start_time = *static_cast<const int*>(value);
            break;
        default:
            break;
    }
}
