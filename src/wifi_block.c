#include <stdio.h>
#include <string.h>

#include <block.h>

static void wifi_block_update(struct block *b) {
	char line[256];

	FILE *state = fopen("/sys/class/net/wlp166s0/operstate", "r");
	if (state == NULL) {
		return;
	}

	fgets(line, sizeof(line), state);
	fclose(state);

	if (strncmp(line, "down", 4) == 0) {
		b->color = 0xff0000;
		strcpy(b->text, "󰖪  down");
		return;
	}

	b->color = 0x00ff00;

	FILE *iwconfig = popen("iwconfig 2>/dev/null", "r");
        if (iwconfig == NULL) {
                return;
        }

	char essid[100];
	int value, max;
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

	snprintf(b->text, BLOCK_BUFFER_SIZE, "  %d%% at %s", value * 100 / max, essid);

	pclose(iwconfig);
}

struct block wifi_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update_after_signal = 0;
	b.update = wifi_block_update;
	b.color = 0xffffff;
	return b;
}
