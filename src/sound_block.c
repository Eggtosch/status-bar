#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <block.h>

static char buf[4096];

static void sound_update(struct block *b) {
	FILE *f = popen("pactl list sinks", "r");
	if (f == NULL) {
		return;
	}

	fread(buf, sizeof(buf), 1, f);

	pclose(f);

	bool muted;
	if (strstr(buf, "Mute: yes") != NULL) {
		b->color = 0xffff00;
		muted = true;
	} else if (strstr(buf, "Mute: no") != NULL) {
		b->color = 0xffffff;
		muted = false;
	} else {
		b->color = 0xff0000;
		muted = true;
	}

	char *volume_line = strstr(buf, "Volume:");
	if (volume_line == NULL) {
		return;
	}

	char *percentage = strchr(volume_line, '%');
	if (percentage == NULL) {
		return;
	}
	percentage--;

	int volume = 0;
	int multiplier = 1;
	while (*percentage >= '0' && *percentage <= '9') {
		volume += (*percentage - '0') * multiplier;
		multiplier *= 10;
		percentage--;
	}

	const char *icon;
	if (muted) {
		icon = "󰖁";
	} else if (volume == 0) {
		icon = "󰕿";
	} else if (volume <= 50) {
		icon = "󰖀";
	} else {
		icon = "󰕾";
	}
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%s %d%%", icon, volume);
}

struct block sound_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update_after_signal = 1;
	b.update = sound_update;
	b.color = 0xffffff;

	sound_update(&b);

	return b;
}
