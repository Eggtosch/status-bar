#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdlib.h>

#include <block.h>

struct block datetime_block_init(void);
struct block ram_block_init(void);
struct block sound_block_init(void);
struct block battery_block_init(void);
struct block brightness_block_init(void);
struct block cpu_block_init(void);
struct block wifi_block_init(void);
struct block eth_block_init(void);
struct block usbeth_block_init(void);
struct block speed_block_init(void);

static const char *notify_level_strs[] = {
	"low", "normal", "critical"
};

void notify(enum notify_urgency level, int timeout, const char *summary, const char *body, ...) {
	pid_t pid = fork();
	if (pid == 0) {
		if (level < NOTIFY_LOW || level > NOTIFY_CRITICAL) {
			level = NOTIFY_NORMAL;
		}

		if (timeout < 0) {
			timeout = 0;
		}

		const char *level_str = notify_level_strs[level];
		char *timeout_str;
		asprintf(&timeout_str, "%d", timeout);
		char *body_str;
		va_list args;
		va_start(args, body);
		vasprintf(&body_str, body, args);
		va_end(args);

		execl(
			"/usr/bin/notify-send",
			"/usr/bin/notify-send",
			"-u", level_str,
			"-t", timeout_str,
			summary, body_str,
			NULL
		);
		exit(0);
	} else {
		waitpid(pid, NULL, 0);
	}
}

const char *hexcolor(uint32_t color) {
	static char buf[8];
	buf[0] = '#';
	snprintf(buf + 1, 7, "%06x", color);
	return buf;
}

static bool got_signal = false;

void sig_handler(int signum) {
	(void) signum;
}

int main(void) {
	signal(SIGUSR1, sig_handler);

	printf("{\"version\":1}\n");
	printf("[\n");

	uint64_t seconds = 0;
	struct block blocks[] = {
		speed_block_init(),
		usbeth_block_init(),
		eth_block_init(),
		wifi_block_init(),
		cpu_block_init(),
		brightness_block_init(),
		battery_block_init(),
		sound_block_init(),
		ram_block_init(),
		datetime_block_init(),
	};
	int numblocks = sizeof(blocks) / sizeof(struct block);

	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;

	while (1) {
		for (int i = 0; i < numblocks; i++) {
			struct block *blk = &blocks[i];
			if (!got_signal && seconds % blk->interval != 0) {
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

		got_signal = false;
		if (nanosleep(&t, &t) != -1) {
			t.tv_sec = 1;
			t.tv_nsec = 0;
			seconds++;
		} else {
			got_signal = true;
		}

		printf(",");
	}
}
