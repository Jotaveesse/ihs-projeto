#ifndef INPUT_PERIPHERAL_H
#define INPUT_PERIPHERAL_H

#include "peripheral.h"

class InputPeripheral : public Peripheral {
public:
    InputPeripheral(int fileDescriptor, unsigned int cmd, unsigned int cnt)
        : Peripheral(fileDescriptor, cmd, cnt) {}

    virtual ~InputPeripheral() {}
};

#endif // INPUT_PERIPHERAL_H