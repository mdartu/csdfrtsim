#ifndef SC_SCHEDULABLE_MODULE_H
#define SC_SCHEDULABLE_MODULE_H

#include <systemc.h>

class sc_scheduler;    // forward declaration

class sc_schedulable_module : public sc_module {
    sc_scheduler *sched;
public:
    sc_out<bool> running;
    sc_schedulable_module(const sc_module_name& nm, sc_scheduler *sched);

protected:
    void wait_ticks(int ticks);
};

#endif // SC_SCHEDULABLE_MODULE_H
