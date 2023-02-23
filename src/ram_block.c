#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <block.h>

static double g_total_kb = -1.0;

static int get_meminfo_entry(const char *entry) {
	FILE *meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL) {
		return -1;
	}

	char line[256];
	int entry_len = strlen(entry);
	while (fgets(line, sizeof(line), meminfo)) {
		if (strncmp(line, entry, entry_len) == 0) {
			fclose(meminfo);
			return atoi(line + entry_len + 1);
		}
	}

	fclose(meminfo);
	return -1;
}

static void cache_total_memory(void) {
	g_total_kb = get_meminfo_entry("MemTotal");
}

static void ram_block_update(struct block *b) {
	double avail_kb = get_meminfo_entry("MemAvailable");
	if (avail_kb == -1) {
		return;
	}

	double used_gb = (g_total_kb - avail_kb) / 1048576.0;
	double total_gb = g_total_kb / 1048576.0;
	snprintf(b->text, BLOCK_BUFFER_SIZE, "%.1f GiB/%.1f GiB", used_gb, total_gb);
}

struct block ram_block_init(void) {
	cache_total_memory();

	struct block b;
	b.interval = 1;
	b.update = ram_block_update;
	b.color = 0xffffff;
	return b;
}
