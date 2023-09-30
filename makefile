CC = gcc
CFLAGS = -Wall -fPIC # -Werror 

SOURCES = main.c utils.c  # Add source files here
OBJECTS = $(SOURCES:.c=.o)
LIBRARY = sorter.so
EXECUTABLE = main

all: $(EXECUTABLE) $(LIBRARY)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(LIBRARY): $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBRARY) $(EXECUTABLE)