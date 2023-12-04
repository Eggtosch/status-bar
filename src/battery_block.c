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

enum charge_state {
	CHARGE_CRITICAL,
	CHARGE_LOW,
	CHARGE_NORMAL
};

static enum charge_state state = CHARGE_NORMAL;
static int charging = 0;

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

	if (discharging && charging == 1) {
		charging = 0;
		notify(NOTIFY_NORMAL, 5000, "Battery", "Battery charger disconnected!");
	} else if (!discharging && charging == 0) {
		charging = 1;
		notify(NOTIFY_NORMAL, 5000, "Battery", "Battery charger connected!");
	}

	if (state == CHARGE_NORMAL && charge_percent < 25) {
		state = CHARGE_LOW;
		notify(NOTIFY_NORMAL, 30000, "Battery", "Low battery level: %.2f%%!", charge_percent);
	} else if (state == CHARGE_LOW && charge_percent < 15) {
		state = CHARGE_CRITICAL;
		notify(NOTIFY_CRITICAL, 0, "Battery", "Critically low battery level: %.2f", charge_percent);
	} else if (state == CHARGE_CRITICAL && charge_percent > 15) {
		state = CHARGE_LOW;
	} else if (state == CHARGE_LOW && charge_percent > 25) {
		state = CHARGE_NORMAL;
	}

	int index = (int) charge_percent / 10;
	if (index < 0) {
		index = 0;
	} else if (index > 9) {
		index = 9;
	}

	const char *icon;
	if (discharging) {
		icon = bat_levels[index];
	} else {
		icon = bat_levels_chrg[index];
	}
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%s %.2f%%", icon, charge_percent);

	fclose(f);
	fclose(f2);
}

struct block battery_block_init(void) {
	cache_charge_full();

	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = battery_update;
	b.color = 0xffffff;
	return b;
}
