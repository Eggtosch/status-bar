#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <block.h>

static void sound_update(struct block *b) {
	FILE *f = popen("amixer -M get Master", "r");

	while (fgetc(f) != '[');

	int volume = 0;
	int c;
	while ((c = fgetc(f)) != '%') {
		volume *= 10;
		volume += c - '0';
	}

	char line[256];
	fgets(line, sizeof(line), f);
	if (strstr(line, "on") != NULL) {
		b->color = 0xffffff;
	} else if (strstr(line, "off") != NULL) {
		b->color = 0xffff00;
	} else {
		b->color = 0xff0000;
	}

	pclose(f);

	snprintf(b->text, BLOCK_BUFFER_SIZE, "%d%%", volume);
}

struct block sound_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update = sound_update;
	b.color = 0xffffff;

	sound_update(&b);

	return b;
}
