#include <iostream>
#include "systemloader.h"
#include "systemvalidator.h"
using namespace std;

int main(int argc, char *argv[]) {
    int ret = 1;
    SystemLoader sl;

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <xml-file>" << endl;
        return -1;
    }

    systemdata::System* system = sl.load(argv[1]);

    if (system) {
        SystemValidator sv(system);
        if (sv.validate()) {
            ret = 0;
        }
    }
    delete system;

    return ret;
}
