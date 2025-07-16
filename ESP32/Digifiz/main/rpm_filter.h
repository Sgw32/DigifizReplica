#ifndef RPM_FILTER_H
#define RPM_FILTER_H
#include <stdint.h>

#define RPM_WINDOW_SIZE 8

typedef struct {
    uint32_t data[RPM_WINDOW_SIZE];
    uint8_t index;
} CircularBuffer;

void init_buffer(CircularBuffer *buffer);
void insert_buffer(CircularBuffer *buffer, int value);
int find_median(CircularBuffer *buffer);
int median_filter(CircularBuffer *buffer, int new_value);

#endif // RPM_FILTER_H
