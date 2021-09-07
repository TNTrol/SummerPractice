CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
PWD=src
SOURCES=$(PWD)/main.c $(PWD)/scanner.c $(PWD)/thread_pool.c $(PWD)/visiting_servers.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=util_scanner
LIBS=-lpthread -lcrypto -lssl

all:$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(PWD)/*.o $(EXECUTABLE)
