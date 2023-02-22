#include <stdio.h>

#include <block.h>

static int g_charge_full = -1;

static void cache_charge_full(void) {
	FILE *f = fopen("/sys/class/power_supply/BAT1/charge_full_design", "r");
	fscanf(f, "%d", &g_charge_full);
	fclose(f);
}

static void battery_update(struct block *b) {
	int charge_now;
	FILE *f = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
	fscanf(f, "%d", &charge_now);

	float charge_percent = charge_now / (double) g_charge_full * 100;
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%.2f%%", charge_percent);
}

struct block battery_block_init(void) {
	cache_charge_full();

	struct block b;
	b.interval = 1;
	b.update = battery_update;
	b.color = 0xffffff;
	return b;
}
