GCC_OPTS = -O3 -D_GNU_SOURCE -std=c99 -Wall
LINK_FLAGS = -lm -lpng -lz


dejong: dejong.c
	gcc $(GCC_OPTS) dejong.c -o dejong $(LINK_FLAGS)
