#include <stdio.h>
#include <string.h>

#include <block.h>

static int connected = 0;

static void usbeth_block_update(struct block *b) {
	char line[256];

	FILE *state = fopen("/sys/class/net/enp0s20f0u5/operstate", "r");
	if (state == NULL) {
		strcpy(b->text, "");
		return;
	}

	fgets(line, sizeof(line), state);
	fclose(state);

	bool down = strncmp(line, "down", 4) == 0;
	if (connected && down) {
		connected = 0;
		notify(NOTIFY_NORMAL, 5000, "USB Ethernet", "USB Ethernet disconnected!");
	} else if (!connected && !down) {
		connected = 1;
		notify(NOTIFY_NORMAL, 5000, "USB Ethernet", "USB Ethernet connected!");
	}

	if (down) {
		strcpy(b->text, " down");
		b->color = 0xff0000;
		return;
	}

	b->color = 0x00ff00;

	FILE *speed = fopen("/sys/class/net/enp0s20f0u5/speed", "r");
	if (speed == NULL) {
		return;
	}

	int mbits;
	fscanf(speed, "%d", &mbits);
	fclose(speed);

	snprintf(b->text, BLOCK_BUFFER_SIZE, " %d Mbit/s", mbits);
}

struct block usbeth_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = usbeth_block_update;
	b.color = 0xffffff;
	return b;
}
