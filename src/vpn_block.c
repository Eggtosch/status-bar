#include <string.h>

#include <block.h>

static void vpn_update(struct block *b) {
	char *iface = iface_get("home-server");
	if (iface == NULL) {
		strcpy(b->text, "");
		b->color = 0xffffff;
	} else {
		strcpy(b->text, "󰌾 home");
		b->color = 0x00ff00;
	}
}

struct block vpn_block_init(void) {
	struct block b;
	b.interval = 5;
	b.update_after_signal = false;
	b.update = vpn_update;
	b.color = 0xffffff;
	return b;
}
