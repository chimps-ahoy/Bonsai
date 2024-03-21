#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t foo(char c)
{
	switch (c) {
		case 'H':
			return 2;
		case 'V':
			return 0;
		case 'L':
			return 0;
		case 'R':
			return 1;
	}
	exit(1);
}

char bar(uint8_t x)
{
	switch (x) {
		case 2:
			return 'H';
		case 0:
			return 'V';
	}
	exit(1);
}

char bar2(uint8_t x)
{
	switch (x) {
		case 0:
			return 'L';
		case 1:
			return 'R';
	}
	exit(1);
}

void split(uint8_t a, uint8_t b)
{
	printf("split(sub[%c],%c)",bar2(a),bar(b));
}

void spawn(uint8_t b) 
{
	printf("spawn(%c)\n",bar2(b));
}

void spawncorner(uint8_t quad, uint8_t s)
{
    static const uint8_t map[4][2] = {
                                        [0b00] = {0b00,0b00},
                                        [0b01] = {0b10,0b01},
                                        [0b10] = {0b01,0b10},
                                        [0b11] = {0b11,0b11}
                                    };
	uint8_t out = map[quad][s>>1];
	split((out&2)>>1,s^2);
	spawn(out&1);
}

void spawncorner_smrt(uint8_t quad, uint8_t s)
{
	uint8_t a = quad&2;
	uint8_t b = (quad&1)<<1;
	split((a&~(b&s))>>1,s^2);
	spawn((b&~(a&s))>>1);
}

int main(int argc, char **argv)
{
	uint8_t ab = atoi(argv[1]);
	uint8_t s = foo(*argv[2]);
	printf("map\n");
	spawncorner(ab, s);
	printf("calc\n");
	spawncorner_smrt(ab, s);
	return 0;
}

/* V=0, H=2   */
/* 00,x = 00,~x
 *
 * 01,x = 00 -> 10,~x
 *        10 -> 01,~x
 *
 * 10,x = 00 -> 01,~x
 *        10 -> 10,~x
 *
 * 11,x = 11,~x
 *
 */
