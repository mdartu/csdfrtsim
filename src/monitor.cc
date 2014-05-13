#include "monitor.h"

Monitor::Monitor() {
    time = 0;
}

int Monitor::get_time() const {
    return time;
}

Monitor::~Monitor() {
}

void Monitor::advance_time() {
    time++;
}

void Monitor::set_parameter(const Task *task, SchedulingParameter param, const void *value) {
}
