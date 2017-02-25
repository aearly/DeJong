GCC_OPTS = -O3 -D_GNU_SOURCE -std=c99 -Wall -Warray-bounds
LINK_FLAGS = -lm -lpng -lz -mavx


dejong: dejong.c
	gcc $(GCC_OPTS) dejong.c -o dejong $(LINK_FLAGS)

.PHONY: clean
clean:
	rm -f dejong
