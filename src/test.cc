#include <iostream>
#include <ctime>
#include <cstdlib>
#include <systemc.h>
#include "system/systemloader.h"
#include "system/systemvalidator.h"
#include "systembuilder.h"
#include "sc_schedulable_module.h"
#include "sc_scheduler.h"
#include "process.h"
#include "statsmonitor.h"
#include "graspmonitor.h"
#include "fifo_fsl.h"
using namespace std;

#define DEFINE_PROCESS(classname, objname, modname, builderobj, schedobj) \
    classname objname(modname, schedobj);\
    builderobj.create_task(&objname);\
    builderobj.set_delays(modname, &objname);

const bool use_fifos = true;

class Psrc : public sc_schedulable_module, public Process {
public:
    sc_in<bool> clk;
    sc_fifo_out<int> OP1;
    sc_fifo_out<int> OP2;
    sc_fifo_out<int> OP3;


    void run() {
        while (true) {
            for (int i = 0; i < 3; i++) {
                switch (i) {
                    case 0:
                        // No reads

                        wait_ticks(exec_delay);

                        wait_ticks(write_delay);
                        if (use_fifos) {
                            OP1.write(1);
                            OP2.write(1);
                        }
                        break;
                    case 1:
                        // No reads

                        wait_ticks(exec_delay);

                        wait_ticks(write_delay);
                        if (use_fifos) {
                            OP1.write(1);
                            OP2.write(1);
                        }

                        break;
                    case 2:
                        // No reads

                        wait_ticks(exec_delay);

                        wait_ticks(write_delay);
                        if (use_fifos) {
                            OP2.write(1);
                            OP3.write(1);
                        }

                        break;
                }
            }
        }
    }

public:
    typedef Psrc SC_CURRENT_USER_MODULE;
    Psrc(const sc_module_name& name, sc_scheduler *sched) : sc_schedulable_module(name, sched) {
        SC_THREAD(run);
            sensitive << clk.pos();
    }
};

class Pf1 : public sc_schedulable_module, public Process {
public:
    sc_in<bool> clk;
    sc_fifo_in<int> IP1;
    sc_fifo_out<int> OP1;

private:
    void run() {
        while (true) {
            wait_ticks(read_delay);
            if (use_fifos) {
                int IP1_value = IP1.read();
            }
            wait_ticks(exec_delay);
            wait_ticks(write_delay);
            if (use_fifos) {
                OP1.write(1);
            }
        }
    }

public:
    typedef Pf1 SC_CURRENT_USER_MODULE;
    Pf1(const sc_module_name& name, sc_scheduler *sched) : sc_schedulable_module(name, sched) {
        SC_THREAD(run);
            sensitive << clk.pos();
    }
};

class Pf2 : public sc_schedulable_module, public Process {
public:
    sc_in<bool> clk;
    sc_fifo_in<int> IP1;
    sc_fifo_out<int> OP1;

    void run() {
        while (true) {
            wait_ticks(read_delay);
            if (use_fifos) {
                int IP1_value = IP1.read();
            }
            wait_ticks(exec_delay);
            wait_ticks(write_delay);
            if (use_fifos) {
                OP1.write(1);
            }
        }
    }

    typedef Pf2 SC_CURRENT_USER_MODULE;
    Pf2(const sc_module_name& name, sc_scheduler *sched) : sc_schedulable_module(name, sched) {
        SC_THREAD(run);
            sensitive << clk.pos();
    }

};

class Psnk : public sc_schedulable_module, public Process {
public:
    sc_in<bool> clk;
    sc_fifo_in<int> IP1;
    sc_fifo_in<int> IP2;
    sc_fifo_in<int> IP3;

    void run() {
        while (true) {
            for (int i = 0; i < 3; i++) {
                switch (i) {
                    case 0:
                        wait_ticks(read_delay);
                        if (use_fifos) {
                            IP1.read();
                            IP2.read();
                        }

                        wait_ticks(exec_delay);
                        break;
                    case 1:
                        wait_ticks(read_delay);
                        if (use_fifos) {
                            IP1.read();
                            IP2.read();
                        }

                        wait_ticks(exec_delay);
                        break;

                    case 2:
                        wait_ticks(read_delay);
                        if (use_fifos) {
                            IP2.read();
                            IP3.read();
                        }

                        wait_ticks(exec_delay);
                        break;
                }
            }
        }
    }

    typedef Psnk SC_CURRENT_USER_MODULE;
    Psnk(const sc_module_name& name, sc_scheduler *sched) : sc_schedulable_module(name, sched) {
        SC_THREAD(run);
            sensitive << clk.pos();
    }
};

int sc_main(int argc, char *argv[])
{
    SystemLoader sl;
    int simulation_time = -1;

    if (argc < 2 || argc > 3) {
        cerr << "Usage: " << argv[0] << " <xml-file> [simulation-time]" << endl;
        return -1;
    } else if (argc == 3) {
        simulation_time = atoi(argv[2]);
    }

    systemdata::System* system = sl.load(argv[1]);
    if (system) {
        SystemValidator sv(system);
        if (!sv.validate()) {
            delete system;
            return -2;
        }
    } else {
        return -3;
    }

    sc_clock clk("sysclk");

    StatsMonitor statsmon;
    GraspMonitor graspmon("trace.grasp");

    sc_scheduler sched("sched");
    sched.add_monitor(&statsmon);
    sched.add_monitor(&graspmon);
    sched.clk(clk);

    SystemBuilder sb(system, &sched);

    sc_trace_file *tf = sc_create_vcd_trace_file("trace");
    sc_trace(tf, clk, "Clock");

    // FIFOs
    fsl<int> E1("E1", sb.get_fifo_size("E1" , 16));
    fsl<int> E2("E2", sb.get_fifo_size("E2" , 16));
    fsl<int> E3("E3", sb.get_fifo_size("E3" , 16));
    fsl<int> E4("E4", sb.get_fifo_size("E4" , 16));
    fsl<int> E5("E5", sb.get_fifo_size("E5" , 16));
    E1.clk(clk);
    E2.clk(clk);
    E3.clk(clk);
    E4.clk(clk);
    E5.clk(clk);

    sc_signal<bool> psrc_running, pf1_running, pf2_running, psnk_running;
    sc_trace(tf, psrc_running, "Psrc_running");
    sc_trace(tf, pf1_running, "Pf1_running");
    sc_trace(tf, pf2_running, "Pf2_running");
    sc_trace(tf, psnk_running, "Psnk_running");

    DEFINE_PROCESS(Psrc, psrc, "Psrc", sb, &sched);
    psrc.clk(clk);
    psrc.OP1(E1);
    psrc.OP2(E3);
    psrc.OP3(E2);
    psrc.running(psrc_running);

    DEFINE_PROCESS(Pf1, pf1, "Pf1", sb, &sched);
    pf1.clk(clk);
    pf1.IP1(E1);
    pf1.OP1(E4);
    pf1.running(pf1_running);

    DEFINE_PROCESS(Pf2, pf2, "Pf2", sb, &sched);
    pf2.clk(clk);
    pf2.IP1(E2);
    pf2.OP1(E5);
    pf2.running(pf2_running);

    DEFINE_PROCESS(Psnk, psnk, "Psnk", sb, &sched);
    psnk.clk(clk);
    psnk.IP1(E4);
    psnk.IP2(E3);
    psnk.IP3(E5);
    psnk.running(psnk_running);

    sb.set_monitoring_parameters(&graspmon);
    sb.set_monitoring_parameters(&statsmon);

    if (simulation_time == -1) {
        simulation_time = sb.get_default_simulation_time();
    }
    cout << "Running simulation for " << simulation_time << " clock cycles" << endl;

    sc_start(simulation_time, SC_NS);

    graspmon.simulation_finished();
    statsmon.simulation_finished();
    statsmon.write_stats(cerr);

    delete system;
    return 0;
}
