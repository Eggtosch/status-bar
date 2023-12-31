#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <block.h>

#define WIFI_TX "/sys/class/net/wlan0/statistics/tx_bytes"
#define WIFI_RX "/sys/class/net/wlan0/statistics/rx_bytes"

#define BUFSIZE 256

static int find_interface(char *tx_file, const char *dir) {
	char *iface = iface_get("enp0s13f0");
	if (iface == NULL) {
		return 0;
	}

	snprintf(tx_file, BUFSIZE, "/sys/class/net/%s/statistics/%s_bytes", iface, dir);
	return 1;
}

const char *eth_stat(bool tx) {
	static char file[BUFSIZE];
	if (find_interface(file, tx ? "tx" : "rx") != 0) {
		return file;
	}

	return "";
}

static uint64_t last_bytes_tx;
static uint64_t last_bytes_rx;

static uint64_t get_statistic(const char *file) {
	FILE *f = fopen(file, "r");
	if (f == NULL) {
		return 0;
	}

	uint64_t value;
	fscanf(f, "%lu", &value);

	fclose(f);
	return value;
}

static const char *get_unit(uint64_t value) {
	if (value < 1024 * 1024) {
		return "KiB/s";
	} else if (value < 1024 * 1024 * 1024) {
		return "MiB/s";
	} else {
		return "GiB/s";
	}
}

static double normalize(uint64_t value) {
	if (value < 1024) {
		return 0.0;
	} else if (value < 1024 * 1024) {
		return value / 1024.0;
	} else if (value < 1024 * 1024 * 1024) {
		return value / (1024.0 * 1024.0);
	} else {
		return value / (1024.0 * 1024.0 * 1024.0);
	}
}

static void speed_block_update(struct block *b) {
	uint64_t bytes_tx = get_statistic(WIFI_TX) + get_statistic(eth_stat(true));
	uint64_t bytes_rx = get_statistic(WIFI_RX) + get_statistic(eth_stat(false));

	uint64_t diff_tx = bytes_tx - last_bytes_tx;
	uint64_t diff_rx = bytes_rx - last_bytes_rx;

	last_bytes_tx = bytes_tx;
	last_bytes_rx = bytes_rx;

	double tx_d = normalize(diff_tx);
	double rx_d = normalize(diff_rx);
	const char *tx_u = get_unit(diff_tx);
	const char *rx_u = get_unit(diff_rx);

	snprintf(b->text, BLOCK_BUFFER_SIZE, "󰕒 %5.1f %s 󰇚 %5.1f %s", tx_d, tx_u, rx_d, rx_u);
}

struct block speed_block_init(void) {
	last_bytes_tx = get_statistic(WIFI_TX) + get_statistic(eth_stat(true));
	last_bytes_rx = get_statistic(WIFI_RX) + get_statistic(eth_stat(false));

	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = speed_block_update;
	b.color = 0xffffff;
	return b;
}
