#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <block.h>

static void eth_block_update(struct block *b) {
	char operstate_file[] = "/sys/class/net/enp0s13f0u1/operstate";
	char speed_file[] = "/sys/class/net/enp0s13f0u1/speed";
	for (int i = 0; i < 4; i++) {
		operstate_file[25] = '1' + i;
		speed_file[25] = '1' + i;
		if (access(operstate_file, F_OK) == 0) {
			break;
		}
	}

	char line[256];

	FILE *state = fopen(operstate_file, "r");
	if (state == NULL) {
		strcpy(b->text, "");
		return;
	}

	fgets(line, sizeof(line), state);
	fclose(state);

	if (strncmp(line, "down", 4) == 0) {
		strcpy(b->text, "󰌙 down");
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

	snprintf(b->text, BLOCK_BUFFER_SIZE, "󰌘 %d Mbit/s", mbits);
}

struct block eth_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = eth_block_update;
	b.color = 0xffffff;
	return b;
}
