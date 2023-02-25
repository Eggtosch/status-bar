#include <stdio.h>
#include <stdint.h>

#include <block.h>

#define WIFI_TX "/sys/class/net/wlp166s0/statistics/tx_bytes"
#define WIFI_RX "/sys/class/net/wlp166s0/statistics/rx_bytes"
#define ETH_TX "/sys/class/net/enp0s13f0u3c2/statistics/tx_bytes"
#define ETH_RX "/sys/class/net/enp0s13f0u3c2/statistics/rx_bytes"

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
	uint64_t bytes_tx = get_statistic(WIFI_TX) + get_statistic(ETH_TX);
	uint64_t bytes_rx = get_statistic(WIFI_RX) + get_statistic(ETH_RX);

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
	last_bytes_tx = get_statistic(WIFI_TX) + get_statistic(ETH_TX);
	last_bytes_rx = get_statistic(WIFI_RX) + get_statistic(ETH_RX);

	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = speed_block_update;
	b.color = 0xffffff;
	return b;
}
