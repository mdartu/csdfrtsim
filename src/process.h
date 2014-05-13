#ifndef PROCESS_H
#define PROCESS_H

class Process {
protected:
    int read_delay;
    int exec_delay;
    int write_delay;

public:
    void set_delays(int read_delay, int exec_delay, int write_delay) {
        this->read_delay = read_delay;
        this->exec_delay = exec_delay;
        this->write_delay = write_delay;
    }
};

#endif // PROCESS_H
