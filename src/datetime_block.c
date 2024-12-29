#include <string.h>
#include <time.h>

#include <block.h>

static void datetime_update(struct block *b) {
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	strftime(b->text, BLOCK_BUFFER_SIZE, "KW %V: %d.%m.%Y %H:%M:%S", tm);
}

struct block datetime_block_init(void) {
	struct block b;
	b.interval = 1;
	b.update_after_signal = false;
	b.update = datetime_update;
	b.color = 0xffffff;
	return b;
}
