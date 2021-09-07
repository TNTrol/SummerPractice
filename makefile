CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.c scanner.c thread_pool.c visiting_servers.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=util_scanner
LIBS=-lpthread -lcrypto -lssl

all:$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o
