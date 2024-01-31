#include <stdio.h>
#include <string.h>

#include <block.h>

enum wifi_status {
	WIFI_CONNECTED,
	WIFI_DISCONNECTED,
	WIFI_AIRPLANE
};

static enum wifi_status status = WIFI_DISCONNECTED;

static void wifi_block_update(struct block *b) {
	char line[256];

	bool airplane_mode = false;
	FILE *rfkill = fopen("/sys/class/rfkill/rfkill1/soft", "r");
	if (rfkill != NULL) {
		int val;
		fscanf(rfkill, "%d", &val);
		airplane_mode = val == 1;
		fclose(rfkill);
	}

	if (airplane_mode && status != WIFI_AIRPLANE) {
		status = WIFI_AIRPLANE;
		notify(NOTIFY_NORMAL, 5000, "Wifi", "Entering airplane mode!");
	} else if (!airplane_mode && status == WIFI_AIRPLANE) {
		status = WIFI_DISCONNECTED;
		notify(NOTIFY_NORMAL, 5000, "Wifi", "Leaving airplane mode!");
	}

	if (airplane_mode) {
		b->color = 0xff0000;
		strcpy(b->text, "󰀝 airplane mode");
		return;
	}

	FILE *state = fopen("/sys/class/net/wlan0/operstate", "r");
	if (state == NULL) {
		return;
	}

	fgets(line, sizeof(line), state);
	fclose(state);

	bool monitor_mode = strncmp(line, "dormant", 7) == 0;
	if (monitor_mode) {
		b->color = 0x00ff00;
		strcpy(b->text, "  monitor mode");
		return;
	}

	bool down = strncmp(line, "down", 4) == 0;
	if (down && status != WIFI_DISCONNECTED) {
		status = WIFI_DISCONNECTED;
		notify(NOTIFY_NORMAL, 5000, "Wifi", "Wifi disconnected!");
	}

	if (down) {
		b->color = 0xff0000;
		strcpy(b->text, "󰖪 down");
		return;
	}

	b->color = 0x00ff00;

	FILE *iwconfig = popen("iwconfig 2>/dev/null", "r");
	if (iwconfig == NULL) {
		return;
	}

	char essid[100];
	int value = 1;
	int max = 1;
	while (fgets(line, sizeof(line), iwconfig)) {
		char *essid_loc = strstr(line, "ESSID");
		if (essid_loc != NULL) {
			essid_loc += 7;
			int i;
			for (i = 0; *essid_loc != '"'; i++) {
				essid[i] = *essid_loc;
				essid_loc++;
			}
			essid[i] = '\0';
		}

		char *link_quality = strstr(line, "Link Quality");
		if (link_quality != NULL) {
			sscanf(link_quality, "Link Quality=%d/%d", &value, &max);
			break;
		}
	}

	if (status == WIFI_DISCONNECTED) {
		status = WIFI_CONNECTED;
		notify(NOTIFY_NORMAL, 5000, "Wifi", "Wifi connected to %s", essid);
	}

	snprintf(b->text, BLOCK_BUFFER_SIZE, " %d%% at %s", value * 100 / max, essid);

	pclose(iwconfig);
}

struct block wifi_block_init(void) {
	struct block b;
	b.interval = 2;
	b.update_after_signal = false;
	b.update = wifi_block_update;
	b.color = 0xffffff;
	return b;
}
