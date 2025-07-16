#include "rpm_filter.h"

void init_buffer(CircularBuffer *buffer) {
    for (int i = 0; i < RPM_WINDOW_SIZE; i++) {
        buffer->data[i] = 0;
    }
    buffer->index = 0;
}

void insert_buffer(CircularBuffer *buffer, int value) {
    buffer->data[buffer->index] = value;
    buffer->index = (buffer->index + 1) % RPM_WINDOW_SIZE;
}

int find_median(CircularBuffer *buffer) {
    int sorted[RPM_WINDOW_SIZE];
    for (int i = 0; i < RPM_WINDOW_SIZE; i++) {
        sorted[i] = buffer->data[i];
    }
    for (int i = 1; i < RPM_WINDOW_SIZE; i++) {
        int key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j = j - 1;
        }
        sorted[j + 1] = key;
    }
    return sorted[RPM_WINDOW_SIZE / 2];
}

int median_filter(CircularBuffer *buffer, int new_value) {
    insert_buffer(buffer, new_value);
    return find_median(buffer);
}
