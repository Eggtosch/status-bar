#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include <block.h>

struct block datetime_block_init(void);
struct block ram_block_init(void);
struct block sound_block_init(void);
struct block battery_block_init(void);
struct block brightness_block_init(void);
struct block cpu_block_init(void);

const char *hexcolor(uint32_t color) {
	static char buf[8];
	buf[0] = '#';
	snprintf(buf + 1, 7, "%06x", color);
	return buf;
}

static int got_signal = 0;

void sig_handler(int signum) {
	(void) signum;
	got_signal = 1;
}

int main(void) {
	signal(SIGUSR1, sig_handler);

	printf("{\"version\":1}\n");
	printf("[\n");

	uint64_t seconds = 0;
	struct block blocks[] = {
		cpu_block_init(),
		brightness_block_init(),
		battery_block_init(),
		sound_block_init(),
		ram_block_init(),
		datetime_block_init(),
	};
	int numblocks = sizeof(blocks) / sizeof(struct block);

	while (1) {
		for (int i = 0; i < numblocks; i++) {
			struct block *blk = &blocks[i];
			if (seconds % blk->interval != 0) {
				continue;
			}
			if (got_signal && !blk->update_after_signal) {
				continue;
			}

			blk->update(blk);

			if (blk->color != 0xffffff) {
				const char *fmt_str = "{\"color\":\"%s\",\"full_text\":\"%s\"}";
				snprintf(blk->buffer, BLOCK_BUFFER_SIZE, fmt_str, hexcolor(blk->color), blk->text);
			} else {
				const char *fmt_str = "{\"full_text\":\"%s\"}";
				snprintf(blk->buffer, BLOCK_BUFFER_SIZE, fmt_str, blk->text);
			}
		}

		printf("[");

		for (int i = 0; i < numblocks; i++) {
			printf("%s,", blocks[i].buffer);
		}

		printf("]\n");
		fflush(stdout);

		got_signal = 0;
		sleep(1);
		if (!got_signal) {
			seconds++;
		}

		printf(",");
	}
}
