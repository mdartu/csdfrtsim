#include "sc_schedulable_module.h"
#include "sc_scheduler.h"

sc_schedulable_module::sc_schedulable_module(const sc_module_name& nm, sc_scheduler *sched) : sched(sched) {
}

void sc_schedulable_module::wait_ticks(int ticks) {
    //cout << "(" << name() << ") Simulating work for " << ticks << " ticks" << endl;
    int total_ticks = static_cast<double>(ticks);
    while (total_ticks > 0) {
        // Wait for permission from scheduler
        running.write(false);
        wait(sched->run_event(this));

        // Simulate work
        running.write(true);
        wait(1, SC_NS);
        running.write(false);

        total_ticks--;
    }
}
