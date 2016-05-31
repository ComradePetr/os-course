CC ?= gcc
LD ?= gcc

CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding \
	-Wall -Wextra -Werror -pedantic -std=c99 \
	-Wframe-larger-than=4096 -Wstack-usage=4096 -Wno-unknown-warning-option
LFLAGS := -nostdlib -z max-page-size=0x1000

SRC := main.c
OBJ := $(SRC:.c=.o)
DEP := $(SRC:.c=.d)

all: main

main: $(OBJ) main.ld
	$(LD) $(LFLAGS) -T main.ld -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEP)

.PHONY: clean
clean:
	rm -f main $(OBJ) $(DEP)
