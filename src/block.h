#pragma once

#include <stdint.h>

#define BLOCK_BUFFER_SIZE 128

struct block {
        int interval;
        void (*update)(struct block*);

        uint32_t color;
        char text[BLOCK_BUFFER_SIZE];
        char buffer[BLOCK_BUFFER_SIZE];
};
