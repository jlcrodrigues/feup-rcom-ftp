TARGET = download
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
ARG1 = ftp://ftp.up.pt/pub/

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

run: ${TARGET}
	./${TARGET} ${ARGS} ${ARG1}

clean:
	-rm -f *.o
	-rm -f $(TARGET)