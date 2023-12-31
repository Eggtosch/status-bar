#include <stdio.h>
#include <string.h>

#include <block.h>

#define BUFSIZE 256

static int connected = 0;

static int find_interface(char *operstate_file, char *speed_file) {
	char *iface = iface_get("enp0s20f0");
	if (iface == NULL) {
		return 0;
	}

	snprintf(operstate_file, BUFSIZE, "/sys/class/net/%s/operstate", iface);
	snprintf(speed_file, BUFSIZE, "/sys/class/net/%s/speed", iface);
	return 1;
}

static void usbeth_block_update(struct block *b) {
	char line[BUFSIZE];
	char operstate_file[BUFSIZE];
	char speed_file[BUFSIZE];

	if (find_interface(operstate_file, speed_file) == 0) {
		if (connected) {
			connected = 0;
			notify(NOTIFY_NORMAL, 5000, "USB Ethernet", "USB Ethernet disconnected!");
		}
		strcpy(b->text, "");
		return;
	}

	FILE *state = fopen(operstate_file, "r");
	if (state == NULL) {
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

	FILE *speed = fopen(speed_file, "r");
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
	b.interval = 5;
	b.update_after_signal = false;
	b.update = usbeth_block_update;
	b.color = 0xffffff;
	return b;
}
