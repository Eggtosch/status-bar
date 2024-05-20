#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <block.h>

static char buf_volume[256];
static char buf_mute[256];
static char buf_type[256];

static bool bt_connected = 0;
static bool usb_connected = 0;

static bool read_from_proc(const char *command, char *buf, int size) {
	FILE *f = popen(command, "r");
	if (f == NULL) {
		return false;
	}
	fread(buf, size, 1, f);
	pclose(f);
	return true;
}

static void sound_update(struct block *b) {
	if (!read_from_proc("pactl get-default-sink", buf_type, sizeof(buf_type))) {
		return;
	}
	if (!read_from_proc("pactl get-sink-volume @DEFAULT_SINK@", buf_volume, sizeof(buf_volume))) {
		return;
	}
	if (!read_from_proc("pactl get-sink-mute @DEFAULT_SINK@", buf_mute, sizeof(buf_mute))) {
		return;
	}

	bool bluetooth = strstr(buf_type, "bluez") != NULL;
	const char *bluetooth_symbol;
	if (bluetooth) {
		if (!bt_connected) {
			bt_connected = 1;
			notify(NOTIFY_NORMAL, 5000, "Audio", "Bluetooth device connected!");
		}
		bluetooth_symbol = "󰂱 ";
	} else {
		if (bt_connected) {
			bt_connected = 0;
			notify(NOTIFY_NORMAL, 5000, "Audio", "Bluetooth device disconnected!");
		}
		bluetooth_symbol = "";
	}

	bool usb = strstr(buf_type, "usb") != NULL;
	const char *usb_symbol;
	if (usb) {
		if (!usb_connected) {
			usb_connected = 1;
			notify(NOTIFY_NORMAL, 5000, "Audio", "Audio now playing over usb");
		}
		usb_symbol = "󰕓 ";
	} else {
		if (usb_connected) {
			usb_connected = 0;
			notify(NOTIFY_NORMAL, 5000, "Audio", "Audio over usb disconnected");
		}
		usb_symbol = "";
	}

	bool muted;
	if (strstr(buf_mute, "Mute: yes") != NULL) {
		b->color = 0xffff00;
		muted = true;
	} else if (strstr(buf_mute, "Mute: no") != NULL) {
		if (bluetooth) {
			b->color = 0x2a9df4;
		} else {
			b->color = 0xffffff;
		}
		muted = false;
	} else {
		b->color = 0xff0000;
		muted = true;
	}

	char *volume_line = strstr(buf_volume, "Volume:");
	if (volume_line == NULL) {
		return;
	}

	char *percentage = strchr(volume_line, '%');
	if (percentage == NULL) {
		return;
	}
	percentage--;

	int volume = 0;
	int multiplier = 1;
	while (*percentage >= '0' && *percentage <= '9') {
		volume += (*percentage - '0') * multiplier;
		multiplier *= 10;
		percentage--;
	}

	const char *icon;
	if (muted) {
		icon = "󰖁";
	} else if (volume == 0) {
		icon = "󰕿";
	} else if (volume <= 50) {
		icon = "󰖀";
	} else {
		icon = "󰕾";
	}

	const char *padding = volume < 10 ? " " : "";
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%s%s%s %s%d%%", usb_symbol, bluetooth_symbol, icon, padding, volume);
}

static void *pactl_subscribe(void *vptr_pid) {
	pid_t pid = (pid_t)(long) vptr_pid;
	FILE *pactl = popen("pactl subscribe", "r");
	if (pactl == NULL) {
		return NULL;
	}

	ssize_t read = 0;
	char *line = NULL;
	size_t n = 0;

	while ((read = getline(&line, &n, pactl)) != -1) {
		if (strstr(line, "on sink #") != NULL) {
			kill(pid, SIGUSR1);
		}
	}

	return NULL;
}

struct block sound_block_init(void) {
	pthread_t pactl_subscribe_thread = 0;
	pthread_create(&pactl_subscribe_thread, NULL, pactl_subscribe, (void*)(long) getpid());

	struct block b;
	b.interval = 5;
	b.update_after_signal = true;
	b.update = sound_update;
	b.color = 0xffffff;

	sound_update(&b);

	return b;
}
