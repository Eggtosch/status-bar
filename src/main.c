#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <block.h>

struct block datetime_block_init(void);
struct block ram_block_init(void);

const char *hexcolor(uint32_t color) {
	static char buf[8];
	buf[0] = '#';
	snprintf(buf + 1, 7, "%06x", color);
	return buf;
}

int main(void) {
	printf("{\"version\":1}\n");
	printf("[\n");

	uint64_t seconds = 0;
	struct block blocks[] = {
		datetime_block_init(),
		ram_block_init(),
	};
	int numblocks = sizeof(blocks) / sizeof(struct block);

	while (1) {
		for (int i = 0; i < numblocks; i++) {
			struct block *blk = &blocks[i];
			if (seconds % blk->interval != 0) {
				continue;
			}
			blk->update(blk);

			const char *fmt_str = "{\"color\":\"%s\",\"full_text\":\"%s\"}";
			snprintf(blk->buffer, BLOCK_BUFFER_SIZE, fmt_str, hexcolor(blk->color), blk->text);
		}

		printf("[");

		for (int i = 0; i < numblocks; i++) {
			printf("%s,", blocks[i].buffer);
		}

		printf("]\n");
		fflush(stdout);

		sleep(1);
		seconds++;

		printf(",");
	}
}
