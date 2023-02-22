#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <block.h>

static void sound_update(struct block *b) {
	FILE *f = popen("amixer get Master", "r");

	while (fgetc(f) != '[');

	int volume = 0;
	int c;
	while ((c = fgetc(f)) != '%') {
		volume *= 10;
		volume += c - '0';
	}

	while (fgetc(f) != '[');

	char buf[4];
	fgets(buf, 4, f);
	bool muted = false;
	if (strncmp(buf, "off", 3) == 0) {
		muted = true;
	}

	snprintf(b->text, BLOCK_BUFFER_SIZE, "%d%%", volume);
	b->color = muted ? 0xffff00 : 0xffffff;
}

struct block sound_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update = sound_update;
	b.color = 0xffffff;

	sound_update(&b);

	return b;
}
