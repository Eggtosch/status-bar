#include <stdio.h>
#include <stdlib.h>

#include <block.h>

static int max_brightness = -1;

static void cache_max_brightness(void) {
	FILE *f = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");
	if (f == NULL) {
		return;
	}

	char line[32];
	fgets(line, sizeof(line), f);

	max_brightness = atoi(line);

	fclose(f);
}

static void brightness_block_update(struct block *b) {
	FILE *f = fopen("/sys/class/backlight/intel_backlight/brightness", "r");
	if (f == NULL) {
		return;
	}

	char line[32];
	fgets(line, sizeof(line), f);

	int brightness = atoi(line);
	int percentage = brightness * 100 / max_brightness;

	snprintf(b->text, BLOCK_BUFFER_SIZE, "󰃠 %d%%", percentage);

	fclose(f);
}

struct block brightness_block_init(void) {
	cache_max_brightness();

	struct block b;
	b.interval = 15;
	b.update_after_signal = true;
	b.update = brightness_block_update;
	b.color = 0xffffff;
	return b;
}
