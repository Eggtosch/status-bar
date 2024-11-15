#include <string.h>

#include <block.h>

static void vpn_update(struct block *b) {
	char *home_iface = iface_get("home-server");
	if (home_iface != NULL) {
		strcpy(b->text, "󰌾 home");
		b->color = 0x00ff00;
		return;
	}

	char *work_iface = iface_get("work");
	if (work_iface != NULL) {
		strcpy(b->text, "󰌾 work");
		b->color = 0x00ff00;
		return;
	}

	strcpy(b->text, "");
	b->color = 0xffffff;
}

struct block vpn_block_init(void) {
	struct block b;
	b.interval = 5;
	b.update_after_signal = false;
	b.update = vpn_update;
	b.color = 0xffffff;
	return b;
}
