#include <string.h>
#include <block.h>

static void datetime_update(struct block *b) {
	strcpy(b->text, "Hello world");
}

struct block datetime_block_init(void) {
	struct block b;
	b.interval = 2;
	b.update = datetime_update;
	b.color = 0xffffff;
	return b;
}
