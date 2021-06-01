#include <time.h>
#include <string.h>

#include "chip8.h"
#include "ui.h"

#define GET_X(opcode)   (((opcode) & 0x0f00) >> 8)
#define GET_Y(opcode)   (((opcode) & 0x00f0) >> 4)
#define GET_N(opcode)   (opcode & 0x000f)
#define GET_NN(opcode)  (opcode & 0x00ff)
#define GET_NNN(opcode) (opcode & 0x0fff)

static bool chip8_init(chip8_t *c8, FILE *rom)
{
    size_t readed_size;

	const uint8_t fontset[] = { 
		0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
		0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
		0x90, 0x90, 0xf0, 0x10, 0x10, // 4
		0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
		0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
		0xf0, 0x10, 0x20, 0x40, 0x40, // 7
		0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
		0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
		0xf0, 0x90, 0xf0, 0x90, 0x90, // a
		0xe0, 0x90, 0xe0, 0x90, 0xe0, // b
		0xf0, 0x80, 0x80, 0x80, 0xf0, // c
		0xe0, 0x90, 0x90, 0x90, 0xe0, // d
		0xf0, 0x80, 0xf0, 0x80, 0xf0, // e
		0xf0, 0x80, 0xf0, 0x80, 0x80  // f
	};

    static const uint8_t keypad[] = {
        SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7,
        SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_A, SDL_SCANCODE_B,
        SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E, SDL_SCANCODE_F,
    };

	memset(c8->memory, 0, MEMORY_SIZE);
	memcpy(c8->memory, fontset, sizeof(fontset));
    memcpy(c8->keypad, keypad, sizeof(keypad));
	readed_size = fread(c8->memory + MEMORY_START_LOCATION, 1, 
                        MEMORY_SIZE - MEMORY_START_LOCATION, rom);

    memset(c8->graph, 0, sizeof(c8->graph));
	memset(c8->regV, 0, sizeof(c8->regV));
	memset(c8->stack, 0, sizeof(c8->stack));

	c8->regI        = 0;
	c8->delay_timer = 0;
	c8->sound_timer = 0;
	c8->sp          = 0;
	c8->pc          = MEMORY_START_LOCATION;
	c8->draw_now    = false;

    return (MEMORY_SIZE - MEMORY_START_LOCATION) >= readed_size;
}

static void chip8_cycle(chip8_t *c8)
{
    size_t i, j, index;

	uint16_t opcode = c8->memory[c8->pc] << 8 | c8->memory[c8->pc+1];

    uint8_t x, y, rnd, height, pixel;

    const uint8_t *key_state;

    switch (opcode & 0xf000) {
    case 0x0000:
        switch (opcode & 0x000f) {
        /* Clears the screen */
        case 0x0000:
            memset(c8->graph, 0, sizeof(c8->graph));
            c8->pc += 2;
            break;
        /* Returns from a subroutine */
        case 0x000e:
            c8->pc = c8->stack[--c8->sp];
            break;
        default:
            fprintf(stderr, "Unknown code 0x%x decoded\n", opcode);
            exit(1);
        }
        break;
    /* Jumps to the Address NNN */
    case 0x1000:
        c8->pc = GET_NNN(opcode);
        break;
    /* Calls subroutine at address NNN */
    case 0x2000:
        c8->stack[c8->sp++] = c8->pc + 2;
        c8->pc = GET_NNN(opcode);
        break;
    /* Skips next instruction if Vx == NN */
    case 0x3000:
        x = GET_X(opcode);

        if (c8->regV[x] == GET_NN(opcode))
            c8->pc += 4;
        else
            c8->pc += 2;
        break;
    /* Skips next instruction if Vx != NN */
    case 0x4000:
        x = GET_X(opcode);

        if (c8->regV[x] != GET_NN(opcode))
            c8->pc += 4;
        else
            c8->pc += 2;
        break;
    /* Skips next instruction if Vx == Vy */
    case 0x5000:
        x = GET_X(opcode);
        y = GET_Y(opcode);

        if (c8->regV[x] == c8->regV[y])
            c8->pc += 4;
        else
            c8->pc += 2;
        break;
    /* Sets Vx = NN */
    case 0x6000:
        x = GET_X(opcode);
        c8->regV[x] = GET_NN(opcode);
        c8->pc += 2;
        break;
    /* Sets Vx += NN */
    case 0x7000:
        x = GET_X(opcode);
        c8->regV[x] += GET_NN(opcode);
        c8->pc += 2;
        break;
    case 0x8000:
        x = GET_X(opcode);
        y = GET_Y(opcode);

        switch (opcode & 0x000f) {
        /* Sets Vx = Vy */
        case 0x0000:
            c8->regV[x] = c8->regV[y];
            break;
        /* Sets Vx |= Vy */
        case 0x0001:
            c8->regV[x] |= c8->regV[y];
            break;
        /* Sets Vx &= Vy */
        case 0x0002:
            c8->regV[x] &= c8->regV[y];
            break;
        /* Sets Vx ^= Vy */
        case 0x0003:
            c8->regV[x] ^= c8->regV[y];
            break;
        /* Sets Vx += Vy, Vf = 1 if has a carry, else 0 */
        case 0x0004:
            if ((int) c8->regV[x] + (int) c8->regV[y] > 255)
                c8->regV[0xf] = 1;
            else
                c8->regV[0xf] = 0;
            c8->regV[x] += c8->regV[y];
            break;
        /* Sets Vx -= Vy, Vf = 0 if has a borrow, else 1 */
        case 0x0005:
            if ((int) c8->regV[x] - (int) c8->regV[y] < 0)
                c8->regV[0xf] = 0;
            else
                c8->regV[0xf] = 1;
            c8->regV[x] -= c8->regV[y];
            break;
        /* Saves LSB of Vx to Vf and then Vx >>= 1 */
        case 0x0006:
            c8->regV[0xf] = c8->regV[x] & 1;
            c8->regV[x] >>= 1;
            break;
        /* Sets Vx = Vy - Vx, Vf = 0 if has a borrow, else 1 */
        case 0x0007:
            if ((int) c8->regV[y] - (int) c8->regV[x] < 0)
                c8->regV[0xf] = 0;
            else
                c8->regV[0xf] = 1;
            c8->regV[x] = c8->regV[y] - c8->regV[x];
            break;
        /* Saves RSB of Vx to Vf and then Vx <<= 1 */
        case 0x000e:
            c8->regV[0xf] = c8->regV[x] >> 7;
            c8->regV[x] <<= 1;
            break;
        default:
            fprintf(stderr, "Unknown code 0x%x decoded\n", opcode);
            exit(1);
        }
        c8->pc += 2;
        break;
    /* Skips next instruction if Vx != Vy */
    case 0x9000:
        x = GET_X(opcode);
        y = GET_Y(opcode);

        if (c8->regV[x] != c8->regV[y])
            c8->pc += 4;
        else
            c8->pc += 2;
        break;
    /* Sets I = Address NNN */
    case 0xa000:
        c8->regI = GET_NNN(opcode);
        c8->pc += 2;
        break;
    /* Sets PC = V0 + Address NNN */
    case 0xb000:
        c8->pc = c8->regV[0] + GET_NNN(opcode);
        break;
    
    /* Sets Vx = rand(0~255) & NN */
    case 0xc000:
        srand(time(NULL));
        rnd = rand() & 255;
        x   = GET_X(opcode);

        c8->regV[x] = rnd & GET_NN(opcode);
        c8->pc += 2;
        break;
	/* Draw */
    case 0xd000:
        x             = GET_X(opcode);
        y             = GET_Y(opcode);
        height        = GET_N(opcode);
        c8->regV[0xf] = 0;

        for (i = 0; i < height; i++) {
            pixel = c8->memory[c8->regI + i];
            for (j = 0; j < 8; j++) {
                if (pixel & (0x80 >> j)) {
                    index = c8->regV[x]+j + (c8->regV[y]+i)*64;
                    if (c8->graph[index])
                        c8->regV[0xf] = 1;
                    c8->graph[index] ^= 1;
                }
            }
        }
        c8->draw_now = true;
        c8->pc += 2;
        break;
    case 0xe000:
        x    = GET_X(opcode);
        key_state = SDL_GetKeyboardState(NULL);

        switch (opcode & 0x000f) {
        case 0x000e:
            if (key_state[c8->keypad[x]])
                c8->pc += 4;
            else
                c8->pc += 2;
            break;
        case 0x0001:
            if (!key_state[c8->keypad[x]])
                c8->pc += 4;
            else
                c8->pc += 2;
            break;
        default:
            fprintf(stderr, "Unknown code 0x%x decoded\n", opcode);
            exit(1);
        }
        break;
    case 0xf000:
        x = GET_X(opcode);

        switch (opcode & 0x00ff) {
        case 0x0007:
            c8->regV[x] = c8->delay_timer;
            break;
        case 0x000a:
            key_state = SDL_GetKeyboardState(NULL);
            
            for (i = 0; i < 16; i++)
                if (key_state[c8->keypad[i]])
                    c8->regV[x] = i;
            break;
        case 0x0015:
            c8->delay_timer = c8->regV[x];
            break;
        case 0x0018:
            c8->sound_timer = c8->regV[x];
            break;
        case 0x001e:
            c8->regI += c8->regV[x];
            break;
        case 0x0029:
            c8->regI = c8->regV[x] * 5;
            break;
        case 0x0033:
            c8->memory[c8->regI]   = c8->regV[x] / 100;
            c8->memory[c8->regI+1] = (c8->regV[x] / 10) % 10;
            c8->memory[c8->regI+2] = c8->regV[x] % 10;
            break;
        case 0x0055:
            for (i = 0; i <= x; i++)
                c8->memory[c8->regI + i] = c8->regV[i];
            break;
        case 0x0065:
            for (i = 0; i <= x; i++)
                c8->regV[i] = c8->memory[c8->regI + i];
            break;
        default:
            fprintf(stderr, "Unknown code 0x%x decoded\n", opcode);
            exit(1);
        }
        c8->pc += 2;
        break;
    default:
		fprintf(stderr, "Unknown code 0x%x decoded\n", opcode);
        exit(1);
    }
}

void chip8_run(FILE *rom)
{
	chip8_t c8;
	if (!chip8_init(&c8, rom)) {
        fprintf(stderr, "Failed to load rom into memory\n");
        exit(2);
    }

	/* SDL ui setup */
	SDL_Event    event;
	SDL_Window   *window;
	SDL_Renderer *renderer;
	SDL_Texture  *texture;
	if ((ui_init(&window, &renderer, &texture)) < 0) {
        fprintf(stderr, "Failed to initialize ui\n");
        exit(2);
    }

    for (;;) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            break;

        chip8_cycle(&c8);

        if (c8.draw_now)
            ui_draw(&c8, renderer, texture);

        /* Update timer */
        if (c8.delay_timer > 0)
            c8.delay_timer--;
        if (c8.sound_timer > 0)
            c8.sound_timer--;
        SDL_Delay(1.5);
    }

    ui_CLEANUP(window, renderer, texture);
}
