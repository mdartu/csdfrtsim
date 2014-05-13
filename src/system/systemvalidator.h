#ifndef SYSTEMVALIDATOR_H
#define SYSTEMVALIDATOR_H

#include <ostream>
#include <string>
#include <vector>
#include "systemdata.h"

class SystemValidator {
    systemdata::System *system;
    std::ostream *errors;

    bool validateScheduler(systemdata::Scheduler *scheduler);
    bool validateTask(systemdata::Task *task);
    bool validateProcessor(systemdata::Processor *processor);
    bool validateMapping(systemdata::Mapping *mapping);
    bool validateSystem(systemdata::System *system);

    // Helper functions
    bool validateName(const std::string &name);

public:
    SystemValidator(systemdata::System *system);
    bool validate();
};

#endif // SYSTEMVALIDATOR_H
