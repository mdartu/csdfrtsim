#include <iostream>
#include <string>
#include "systemloader.h"
using namespace std;

bool SystemLoader::getAttributeValue(xmlNode *node, const string &attribute_name, string &attribute_value) {
    bool required = true;       // TODO: parameter
    xmlAttrPtr attribute = xmlHasProp(node, BAD_CAST attribute_name.c_str());
    if (!attribute) {
        if (required) {
            cerr << "Missing attribute '" << attribute_name << "' for element '" << node->name << "' on line " << node->line << endl;
        }
        return false;
    }

    // FIXME: this does not care about utf-8 encoding
    attribute_value = reinterpret_cast<char*>(attribute->children->content);
    return true;
}

bool SystemLoader::getAttributeValue(xmlNode *node, const string &attribute_name, int &attribute_value) {
    string value;
    if (!getAttributeValue(node, attribute_name, value)) {
        return false;
    }

    try {
        attribute_value = stoi(value);
    } catch (exception) {
        cerr << "Invalid argument for " << attribute_name << " on line " << node->line << ": " << value << endl;
        return false;
    }

    return true;
}

systemdata::Scheduler *SystemLoader::processScheduler(xmlNode *scheduler_node) {
    systemdata::Scheduler *scheduler;
    string scheduler_name, algorithm_name, type_name, mapping_name;
    systemdata::SchedulerType type;
    systemdata::Algorithm algorithm;

    if (!getAttributeValue(scheduler_node, "name", scheduler_name) ||
        !getAttributeValue(scheduler_node, "algorithm", algorithm_name) ||
        !getAttributeValue(scheduler_node, "type", type_name) ||
        !getAttributeValue(scheduler_node, "mapping", mapping_name)) {
        return NULL;
    }

    if (algorithm_name == "null") {
        algorithm = systemdata::SCHED_NULL;
    } else if (algorithm_name == "static") {
        algorithm = systemdata::SCHED_STATIC;
    } else if (algorithm_name == "EDF") {
        algorithm = systemdata::SCHED_EDF;
    } else {
        cerr << "Invalid algorithm: " << algorithm_name << " for scheduler " << scheduler_name << endl;
        return NULL;
    }

    if (type_name == "global") {
        type = systemdata::SCHEDTYPE_GLOBAL;
    } else if (type_name == "partitioned") {
        type = systemdata::SCHEDTYPE_PARTITIONED;
    } else if (type_name == "hybrid-semipartitioned") {
        type = systemdata::SCHEDTYPE_HYBRID;
    } else {
        cerr << "Invalid scheduler type: " << type_name << " for scheduler " << scheduler_name << endl;
        return NULL;
    }

    return new systemdata::Scheduler(scheduler_name, algorithm, type, mapping_name);
}

systemdata::Task *SystemLoader::processTask(xmlNode *task_node) {
    systemdata::Task *task;
    string task_name, type_name;
    int wcet, read_delay, write_delay, start_time, period, deadline, priority;
    systemdata::TaskType type;

    if (!getAttributeValue(task_node, "name", task_name) ||
        !getAttributeValue(task_node, "wcet", wcet) ||
        !getAttributeValue(task_node, "readDelay", read_delay) ||
        !getAttributeValue(task_node, "writeDelay", write_delay) ||
        !getAttributeValue(task_node, "startTime", start_time) ||
        !getAttributeValue(task_node, "period", period) ||
        !getAttributeValue(task_node, "deadline", deadline) ||
        !getAttributeValue(task_node, "priority", priority) ||
        !getAttributeValue(task_node, "type", type_name)) {
        return NULL;
    }

    if (type_name == "fixed") {
        type = systemdata::TASKTYPE_FIXED;
    } else if (type_name == "migrating") {
        type = systemdata::TASKTYPE_MIGRATING;
    } else {
        cerr << "Invalid task type: " << type_name << " for task " << task_name << endl;
        return NULL;
    }

    return new systemdata::Task(task_name, wcet, read_delay, write_delay, start_time, period, deadline, priority, type);
}

systemdata::Processor *SystemLoader::processProcessor(xmlNode *processor_node) {
    string processor_name, scheduler_name;
    if (!getAttributeValue(processor_node, "name", processor_name) ||
        !getAttributeValue(processor_node, "scheduler", scheduler_name)) {
        return NULL;
    }

    return new systemdata::Processor(processor_name, scheduler_name);
}

systemdata::Fifo *SystemLoader::processFifo(xmlNode *fifo_node) {
    string name;
    int size;
    if (!getAttributeValue(fifo_node, "name", name) ||
        !getAttributeValue(fifo_node, "size", size)) {
        return NULL;
    }

    return new systemdata::Fifo(name, size);
}

systemdata::Mapping *SystemLoader::processMapping(xmlNode *mapping_node) {
    systemdata::Mapping *mapping = new systemdata::Mapping();
    string name;

    if (!getAttributeValue(mapping_node, "name", name)) {
        return NULL;
    }

    mapping->name = name;

    for (xmlNode *pnode = mapping_node->children; pnode; pnode = pnode->next) {
        if (pnode->type != XML_ELEMENT_NODE) continue;

        if (!xmlStrEqual(pnode->name, BAD_CAST "processor")) {
            cerr << "Invalid element '" << pnode->name << "' on line " << pnode->line << endl;
            delete mapping;
            return NULL;
        }

        string processor_name;
        if (!getAttributeValue(pnode, "name", processor_name)) {
            delete mapping;
            return NULL;
        }

        systemdata::Mapping::ProcessorEntry *pentry = new systemdata::Mapping::ProcessorEntry(processor_name);

        for (xmlNode *tnode = pnode->children; tnode; tnode = tnode->next) {
            if (tnode->type != XML_ELEMENT_NODE) continue;

            if (!xmlStrEqual(tnode->name, BAD_CAST "task")) {
                cerr << "Invalid element '" << tnode->name << "' on line " << tnode->line << endl;
                delete mapping;
                return NULL;
            }

            string task_name;
            if (!getAttributeValue(tnode, "name", task_name)) {
                delete mapping;
                return NULL;
            }

            systemdata::Mapping::TaskEntry *task_entry = new systemdata::Mapping::TaskEntry(task_name);
            pentry->tasks.push_back(task_entry);
        }

        mapping->entries.push_back(pentry);
    }

    return mapping;
}

systemdata::System *SystemLoader::load(const string &filename) {
    LIBXML_TEST_VERSION

    xmlDocPtr doc;
    doc = xmlReadFile(filename.c_str(), NULL, 0);
    if (!doc) {
        cerr << "Couldn't open file: " << filename << endl;
        return NULL;
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    systemdata::System *system = new systemdata::System();

    for (xmlNode *node = root->children; node; node = node->next) {
        bool ret;
        if (node->type != XML_ELEMENT_NODE) continue;

        if (xmlStrEqual(node->name, BAD_CAST "scheduler")) {
            ret = system->addScheduler(processScheduler(node));
        } else if (xmlStrEqual(node->name, BAD_CAST "task")) {
            ret = system->addTask(processTask(node));
        } else if (xmlStrEqual(node->name, BAD_CAST "processor")) {
            ret = system->addProcessor(processProcessor(node));
        } else if (xmlStrEqual(node->name, BAD_CAST "fifo")) {
            ret = system->addFifo(processFifo(node));
        } else if (xmlStrEqual(node->name, BAD_CAST "mapping")) {
            ret = system->addMapping(processMapping(node));
        } else {
            cerr << "Unsupported element: '" << node->name << "'" << endl;
            xmlFreeDoc(doc);
            xmlCleanupParser();
            delete system;
            return NULL;
        }

        if (!ret) {
            xmlFreeDoc(doc);
            xmlCleanupParser();
            delete system;
            return NULL;
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return system;
}
