CC:=gcc
CFLAGS?=-Wall -Werror

APPS:=makeg722 makesln16 makesln8 sln8tog722

all: $(APPS)

makeg722: makeg722.o g722_encode.o g722_decode.o
	$(CC) -o $@ $^

sln8tog722: sln8tog722.o g722_encode.o g722_decode.o
	$(CC) -o $@ $^

makesln16: makesln16.o g722_encode.o g722_decode.o
	$(CC) -o $@ $^

makesln8: makesln8.o g722_encode.o g722_decode.o
	$(CC) -o $@ $^

clean:
	rm -f $(APPS) *.o
