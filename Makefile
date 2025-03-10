CFLAGS := -Wall -Wextra -Wimplicit-fallthrough -g -std=c99
LDFLAGS := -lncurses

all: snake

main.o: main.c
	gcc $< -c -o $@ $(CFLAGS) $(LDFLAGS)

snake: main.o
	gcc $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm main.o
	rm snake
