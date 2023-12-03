#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BLOCK_BUFFER_SIZE 128

struct block {
	int interval;
	bool update_after_signal;
	void (*update)(struct block*);

	uint32_t color;
	char text[BLOCK_BUFFER_SIZE];
	char buffer[BLOCK_BUFFER_SIZE];
};

enum notify_urgency {
	NOTIFY_LOW,
	NOTIFY_NORMAL,
	NOTIFY_CRITICAL,
};

void notify(enum notify_urgency level, int timeout, const char *summary, const char *body, ...);
