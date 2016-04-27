#include "common.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void * malloc_(size_t size) {
    void * buffer = malloc(size);
    if (buffer == NULL) {
        char * message = "malloc() failed";
        write(2, message, strlen(message));
        _exit(1);
    }
    return buffer;
}

int check(const char * message, int result) {
    if (result == -1) {
        perror(message);
        _exit(1);
    }
    return result;
}

buffer_t new_buffer(int size) {
    buffer_t buffer;
    buffer.size = size;
    buffer.current_size = 0;
    buffer.buffer = malloc_(size);
    return buffer;
}
