#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>
#include "arg.h"
#include "words.h"

char *getrandomword(void);
uint32_t randomuniform(uint32_t);
uint32_t truerandom(void);

int main(int argc, char** argv) {
    char *argv0;
    static char* opt_word_count = NULL;
    int word_count;

    ARGBEGIN {
        case 'n':
            opt_word_count = ARGF();
            break;
    }
    ARGEND
    
    if(opt_word_count)
        word_count = atoi(opt_word_count);
    else
        word_count = 7;
    
    if(word_count <=0 )
        exit(EXIT_FAILURE);

    while(word_count--) {
        printf( word_count ? "%s " : "%s", getrandomword());
    }
    printf("\n");
    
    return 0;
}

char *getrandomword() {
    return dicewords[randomuniform(DWARE_WORD_COUNT)];
}

uint32_t truerandom(void) {
    unsigned char buf[4];
    uint32_t ret;
    int getrandomret;

    do {
        getrandomret = syscall(SYS_getrandom, buf, 4, NULL);
    } while(getrandomret == -1 && errno == EINTR );

    if(getrandomret != 4 ) {
        perror( "Failed to get randomness!");
        exit(EXIT_FAILURE);
    }   
    ret = buf[0];
    ret |= buf[1] << 8;
    ret |= buf[2] << 16;
    ret |= buf[3] << 24;

    return ret;
}

/*
 * Copied from libbsd https://wiki.freedesktop.org/libbsd/
 *
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
uint32_t randomuniform(uint32_t upper_bound) {
	uint32_t r, min;

	if (upper_bound < 2)
		return (0);

#if (ULONG_MAX > 0xffffffffUL)
	min = 0x100000000UL % upper_bound;
#else
	/* Calculate (2**32 % upper_bound) avoiding 64-bit math */
	if (upper_bound > 0x80000000)
		min = 1 + ~upper_bound;		/* 2**32 - upper_bound */
	else {
		/* (2**32 - (x * 2)) % x == 2**32 % x when x <= 2**31 */
		min = ((0xffffffff - (upper_bound * 2)) + 1) % upper_bound;
	}
#endif

	/*
	 * This could theoretically loop forever but each retry has
	 * p > 0.5 (worst case, usually far better) of selecting a
	 * number inside the range we need, so it should rarely need
	 * to re-roll.
	 */
	for (;;) {
		r = truerandom();
		if (r >= min)
			break;
	}

	return (r % upper_bound);
}
