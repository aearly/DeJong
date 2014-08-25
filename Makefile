GCC_OPTS = -O3 -D_GNU_SOURCE -std=c99 -Wall


dejong: dejong.c
	gcc $(GCC_OPTS) dejong.c -o dejong -lm -lpng -lz
