all: audiofilter

clean:
	rm -f audiofilter.o audiofilter

audiofilter.o: audiofilter.c
	$(CC) -c -o $@ $^

audiofilter: audiofilter.o
	$(CC) -o $@ $^ -lm

FORCE:

.PHONY: all clean
