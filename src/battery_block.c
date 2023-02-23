#include <stdio.h>
#include <string.h>

#include <block.h>

static int g_charge_full = -1;

static const char *bat_levels[] = {
	"󰁺", "󰁻", "󰁼", "󰁽",
	"󰁾", "󰁿", "󰂀", "󰂁",
	"󰂂", "󰁹",
};

static const char *bat_levels_chrg[] = {
	"󰢜", "󰂆", "󰂇", "󰂈",
	"󰢝", "󰂉", "󰢞", "󰂊",
	"󰂋", "󰂅",
};

static void cache_charge_full(void) {
	FILE *f = fopen("/sys/class/power_supply/BAT1/charge_full_design", "r");
	if (f == NULL) {
		return;
	}

	fscanf(f, "%d", &g_charge_full);
	fclose(f);
}

static void battery_update(struct block *b) {
	int charge_now;
	FILE *f = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
	if (f == NULL) {
		return;
	}

	char status[20];
	FILE *f2 = fopen("/sys/class/power_supply/BAT1/status", "r");
	if (f2 == NULL) {
		fclose(f);
		return;
	}

	fscanf(f, "%d", &charge_now);
	fscanf(f2, "%s", status);

	float charge_percent = charge_now / (double) g_charge_full * 100;
	int discharging = strcmp(status, "Discharging") == 0;
	const char *icon;
	if (discharging) {
		icon = bat_levels[(int) charge_percent / 10];
	} else {
		icon = bat_levels_chrg[(int) charge_percent / 10];
	}
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%s %.2f%%", icon, charge_percent);

	fclose(f);
	fclose(f2);
}

struct block battery_block_init(void) {
	cache_charge_full();

	struct block b;
	b.interval = 1;
	b.update_after_signal = 0;
	b.update = battery_update;
	b.color = 0xffffff;
	return b;
}
