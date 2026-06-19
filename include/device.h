#ifndef _device_h
#define _device_h

struct device {
    char name[64];
    void *device_struct;
};

void init_devices();
#endif