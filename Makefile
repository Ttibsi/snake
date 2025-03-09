CFLAGS := -Wall -Wextra -Wimplicit-fallthrough -g
LDFLAGS := -lncurses

all: snake

main.o: main.c
	gcc $< -c -o $@ $(CFLAGS) $(LDFLAGS)

snake: main.o
	gcc $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm main.o
	rm snake
