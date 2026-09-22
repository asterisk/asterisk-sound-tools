export CC

CC:=gcc

BIN:=check_sounds

all: $(BIN)

%.o: %.c
	$(CC) -o $@ -c $<

check_sounds: check_sounds.o

clean:
	rm -f $(BIN)
	rm -f *.o
