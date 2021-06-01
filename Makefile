CFLAGS  = -std=c11 -O2 -Wall -Wextra -DNDEBUG
LDFLAGS = -lSDL2
BIN     = chip-8
OBJS    = chip8.o ui.o

chip-8: $(OBJS)
	$(CC) -o $(BIN) main.c $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS)
