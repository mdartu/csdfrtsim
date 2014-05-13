#ifndef SYSTEMLOADER_H
#define SYSTEMLOADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "systemdata.h"

class SystemLoader {
    bool getAttributeValue(xmlNode *node, const std::string &attribute_name, std::string &attribute_value);
    bool getAttributeValue(xmlNode *node, const std::string &attribute_name, int &attribute_value);
    systemdata::Task *processTask(xmlNode *task_node);
    systemdata::Scheduler *processScheduler(xmlNode *task_node);
    systemdata::Processor *processProcessor(xmlNode *processor_node);
    systemdata::Fifo *processFifo(xmlNode *fifo_node);
    systemdata::Mapping *processMapping(xmlNode *mapping_node);

public:
    systemdata::System *load(const std::string &filename);
};

#endif // SYSTEMLOADER_H
