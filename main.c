#include <stdio.h>

#include "chip8.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: ./chip-8 [ROM]\n");
		return 1;
	}

	FILE *rom = fopen(argv[1], "rb");
	if (rom == NULL) {
		fprintf(stderr, "Failed to open ROM %s\n", argv[1]);
		return 2;
	}

	chip8_run(rom);

    return 0;
}
