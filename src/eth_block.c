#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <block.h>

#define BUFSIZE 256

static int connected = 0;

static int find_interface(char *operstate_file, char *speed_file) {
	DIR *d = opendir("/sys/class/net/");
	struct dirent *de;
	int found = 0;
	while ((de = readdir(d)) != NULL) {
		if (de->d_type == DT_LNK && strstr(de->d_name, "enp0s13f0") != NULL) {
			snprintf(operstate_file, BUFSIZE, "/sys/class/net/%s/operstate", de->d_name);
			snprintf(speed_file, BUFSIZE, "/sys/class/net/%s/speed", de->d_name);
			found = 1;
			break;
		}
	}

	closedir(d);
	return found;
}

static void eth_block_update(struct block *b) {
	char operstate_file[BUFSIZE];
	char speed_file[BUFSIZE];
	if (find_interface(operstate_file, speed_file) == 0) {
		strcpy(b->text, "");
		return;
	}

	char line[256];

	FILE *state = fopen(operstate_file, "r");
	if (state == NULL) {
		if (connected) {
			connected = 0;
			notify(NOTIFY_NORMAL, 5000, "Ethernet", "Ethernet disconnected!");
		}
		strcpy(b->text, "");
		return;
	}

	fgets(line, sizeof(line), state);
	fclose(state);

	bool down = strncmp(line, "down", 4) == 0;
	if (connected && down) {
		connected = 0;
		notify(NOTIFY_NORMAL, 5000, "Ethernet", "Ethernet disconnected!");
	} else if (!connected && !down) {
		connected = 1;
		notify(NOTIFY_NORMAL, 5000, "Ethernet", "Ethernet connected!");
	}

	if (down) {
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
