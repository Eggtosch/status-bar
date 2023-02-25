#include <stdio.h>
#include <string.h>

#include <block.h>

#define CPU_UPDATE_SHORT
#define NCPUS 16

#ifndef CPU_UPDATE_SHORT
static int cpu_last_usage[NCPUS];

static void get_cpu_usage(int *cpus) {
	char line[256];
	FILE *f = fopen("/proc/stat", "r");
	if (f == NULL) {
		for (int i = 0; i < NCPUS; i++) {
			cpus[i] = -1;
		}
		return;
	}

	fgets(line, sizeof(line), f);

	for (int i = 0; i < NCPUS; i++) {
		char _name[10];
		int _1, _2, _3;
		fgets(line, sizeof(line), f);
		sscanf(line, "%s %d %d %d %d", _name, &_1, &_2, &_3, &cpus[i]);
	}

	fclose(f);
}

static void cpu_block_update(struct block *b) {
	int cpu_usages[NCPUS];
	get_cpu_usages(cpu_usages);

	char *text = b->text;
	int bytes = BLOCK_BUFFER_SIZE;
	int total_len = 0;

	for (int i = 0; i < NCPUS; i++) {
		if (cpu_usages[i] == -1) {
			continue;
		}

		int idle = cpu_usages[i] - cpu_last_usage[i];
		int used = idle > 100 ? 0 : 100 - idle;
		cpu_last_usage[i] = cpu_usages[i];

		int len = snprintf(text, bytes, "%02d%% ", used);
		total_len += len;
		bytes -= len;
		text += len;
	}

	b->text[total_len-1] = '\0';
}
#else

static int cpu_last_usage;

static void get_cpu_usage(int *cpu) {
	FILE *f = fopen("/proc/stat", "r");
	if (f == NULL) {
		*cpu = -1;
		return;
	}

	int _1, _2, _3;
	fscanf(f, "cpu %d %d %d %d", &_1, &_2, &_3, cpu);
	fclose(f);
}

static void cpu_block_update(struct block *b) {
	int usage;
	get_cpu_usage(&usage);
	if (usage == -1) {
		cpu_last_usage += 100 * NCPUS;
		return;
	}

	int idle = (usage - cpu_last_usage) / NCPUS;
	int used = idle > 100 ? 0 : 100 - idle;
	cpu_last_usage = usage;

	const char *padding = used == 100 ? "" : " ";
	snprintf(b->text, BLOCK_BUFFER_SIZE, "󰻠%s%02d%%", padding, used);
}

#endif

struct block cpu_block_init(void) {
	get_cpu_usage(&cpu_last_usage);

	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = cpu_block_update;
	b.color = 0xffffff;
	return b;
}
