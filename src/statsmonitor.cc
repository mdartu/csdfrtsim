#include <iostream>
#include "statsmonitor.h"
#include "scheduler.h"
using namespace std;

// Helper function to write table rows to the output stream and take care of the spacing.
static void write_table_row(ostream &stream, const string &s1, const string &s2, const string &s3) {
    const int cols = 20;
    stream << "| " << s1;
    for (int i = 0; i < cols - s1.length() - 1; i++) stream << " ";
    stream << "| " << s2;
    for (int i = 0; i < cols - s2.length() - 1; i++) stream << " ";
    stream << "| " << s3;
    for (int i = 0; i < cols - s3.length() - 1; i++) stream << " ";
    stream << "|" << endl;
}

void StatsMonitor::add_processor(const Processor *p) {
}

void StatsMonitor::add_task(const Task *t) {
}

void StatsMonitor::task_preempted(const Task *t, const Processor *p) {
    task_stats[t].et += get_time() - task_stats[t].last_resume;
    proc_stats[task_stats[t].proc].util += get_time() - task_stats[t].last_resume;
    task_stats[t].last_preempt = get_time();
}

void StatsMonitor::task_resumed(const Task *t, const Processor *p) {
    task_stats[t].last_resume = get_time();
    if (task_stats[t].proc != p) {
        task_stats[t].proc = p;
        task_stats[t].migrations++;
    }
}

void StatsMonitor::simulation_finished() {
    // Update execution time for tasks that were running at the end of the simulation
    for (auto &ts : task_stats) {
        if (ts.second.last_resume >= ts.second.last_preempt) {      // we need to check >=, because a task might be preempted and resumed in the same cycle (migration)
            ts.second.et += get_time() - ts.second.last_resume;
            proc_stats[ts.second.proc].util += get_time() - ts.second.last_resume;
        }
    }
}

void StatsMonitor::write_stats(ostream &stream) {
    stream << "+--------------------+--------------------+--------------------+" << endl
           << "|                       Task statistics                        |" << endl
           << "+--------------------+--------------------+--------------------+" << endl
           << "| Name               | Metric             | Value              |" << endl
           << "+--------------------+--------------------+--------------------+" << endl;
    int total_et = 0;
    int total_migrations = 0;
    for (const auto &t : task_stats) {
        write_table_row(stream, t.first->get_name(), "Execution time", to_string(t.second.et));
        write_table_row(stream, "", "Migrations", to_string(t.second.migrations));
        total_et += t.second.et;
        total_migrations += t.second.migrations;
    }
    stream << "+--------------------+--------------------+--------------------+" << endl;
    write_table_row(stream, "Total", "Execution time", to_string(total_et));
    write_table_row(stream, "", "Migrations", to_string(total_migrations));
    stream << "+--------------------+--------------------+--------------------+" << endl << endl;


    stream << "+--------------------+--------------------+--------------------+" << endl
           << "|                     Processor statistics                     |" << endl
           << "+--------------------+--------------------+--------------------+" << endl
           << "| Name               | Metric             | Value              |" << endl
           << "+--------------------+--------------------+--------------------+" << endl;
    int total_util = 0;
    for (const auto &p : proc_stats) {
        write_table_row(stream, p.first->get_name(), "Utilization", to_string(p.second.util / static_cast<double>(get_time())));
        total_util += p.second.util;
    }
    stream << "+--------------------+--------------------+--------------------+" << endl;
    write_table_row(stream, "Total", "Utilization", to_string(total_util / static_cast<double>(get_time())));
    stream << "+--------------------+--------------------+--------------------+" << endl;
}

StatsMonitor::~StatsMonitor() {
}
