CC     := gcc
RM     := rm -f

LDFLAGS := -pipe -flto

CFILES := src/main.c src/datetime_block.c src/ram_block.c src/sound_block.c \
		  src/battery_block.c src/brightness_block.c src/cpu_block.c \
		  src/wifi_block.c src/eth_block.c src/usbeth_block.c src/speed_block.c

FLAGS  := -Wall -Wextra -Werror -Isrc/ -pipe -O2 -flto -march=native -s -MMD -MP
OBJDIR := bin
BINARY := bin/status-bar
OBJS   := $(CFILES:%.c=$(OBJDIR)/%.o)
HEADER_DEPS := $(CFILES:%.c=$(OBJDIR)/%.d)


.PHONY: all install
all: $(OBJDIR) $(BINARY)

install: all
	killall -9 status-bar
	cp $(BINARY) ~/.config/sway/status-bar
	swaymsg reload

$(OBJDIR):
	mkdir -p $(OBJDIR)/src

$(BINARY): $(OBJS)
	$(CC) $(LDFLAGS) -o $(BINARY) $(OBJS)

-include $(HEADER_DEPS)
$(OBJDIR)/%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@


.PHONY: clean
clean:
	$(RM) $(BINARY)
	$(RM) $(OBJS)
	$(RM) $(HEADER_DEPS)
	$(RM) -r $(OBJDIR)
