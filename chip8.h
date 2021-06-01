#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define MEMORY_START_LOCATION 512

#define GRAPH_W 64
#define GRAPH_H 32

typedef struct chip8 {
    uint8_t memory[MEMORY_SIZE];
    uint8_t graph[GRAPH_H * GRAPH_W];
    
    uint8_t regV[16];
	uint16_t regI;

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t stack[16];
    uint16_t pc;
    uint8_t  sp;

    uint8_t keypad[16];
	bool draw_now;
} chip8_t;

void chip8_run(FILE *rom);

#endif
