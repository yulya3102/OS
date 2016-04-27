#include <stdlib.h>

void * malloc_(size_t size);
int check(const char * message, int result);

typedef struct {
    int size, current_size;
    char * buffer;
} buffer_t;

buffer_t new_buffer(int size);
