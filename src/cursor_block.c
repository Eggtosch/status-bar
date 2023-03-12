#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <block.h>

static void cursor_block_update(struct block *b) {
	bool cursor = access("/home/oskar/.config/sway/data/cursor", F_OK) == 0;

	const char *icon = cursor ? "󰌌" : "󰍽";
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%s", icon);
}

struct block cursor_block_init(void) {
	struct block b;
	b.interval = 60;
	b.update_after_signal = true;
	b.update = cursor_block_update;
	b.color = 0xffffff;
	return b;
}
