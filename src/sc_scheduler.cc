#include <cstdlib>
#include "sc_schedulable_module.h"
#include "sc_scheduler.h"
#include "graspmonitor.h"
#include "statsmonitor.h"

void sc_scheduler::run() {
    wait();         // Ensure that all tasks have had a chance to call wait_ticks()

    for (auto s : schedulers) {
        s->init();
    }
    while (true) {
        if (processors.size() == 0) {
            cerr << "Error: no processors defined" << endl;
            exit(1);
        }

        for (auto s : schedulers) {
            s->run();
        }

        // Run the tasks for each processor
        for (auto p : processors) {

            if (p->current != p->next) {
                // Task preempted / resumed
                for (auto m : monitors) {
                    if (p->current) m->task_preempted(p->current, p);
                    if (p->next) m->task_resumed(p->next, p);
                }
            }

            p->previous = p->current;
            p->current = p->next;
            p->next = NULL;
            if (p->current) {
                p->current->run_event.notify();
            }
        }

        for (auto m : monitors) {
            m->advance_time();
        }

        wait(1, SC_NS);
    }
}

void sc_scheduler::add_monitor(Monitor *monitor) {
    this->monitors.push_back(monitor);
}

sc_scheduler::sc_scheduler(const sc_module_name &name) {
    SC_THREAD(run);
        sensitive << clk.pos();

    counter = 0;
    cout << "sc_scheduler initialized" << endl;
}

sc_scheduler::~sc_scheduler() {
    for (auto t : tasksets) {
        delete t;
    }
}

void sc_scheduler::add_scheduler(Scheduler *sched) {
    this->schedulers.push_back(sched);
}

void sc_scheduler::add_task(Task *task) {
    task_table[task->module] = task;
    tasks.push_back(task);
    for (auto m : monitors) {
        m->add_task(task);
    }
}

void sc_scheduler::add_processor(Processor *processor) {
    processors.push_back(processor);
    for (auto m : monitors) {
        m->add_processor(processor);
    }
}

sc_event& sc_scheduler::run_event(const sc_schedulable_module* mod) {
    auto iter = task_table.find(mod);
    if (iter == task_table.end()) {
        cerr << "Fatal error: Task '" << mod->name() << "' not registered with scheduler" << endl;
        exit(1);
    }

    return iter->second->run_event;
}
