prefix=/opt/local

CC = gcc
DEBUG= -Wall -g
INCL = -I$(prefix)/include -I./
LDFLAGS = -L$(prefix)/lib64 -L./
TARGET = openicc-config-read
CFLAGS=$(DEBUG) $(INCL)
HEADER  = openicc_config.h
SOURCES = \
	openicc_config.c \
	openicc_config_read.c
OBJECTS = $(SOURCES:.c=.o)

all:	$(TARGET)

$(TARGET):	$(OBJECTS) $(SOURCES) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) -lyajl

clean:
	$(RM) $(OBJECTS)

check:	$(TARGET)
	./$(TARGET) ../OpenICC_device_config_DB.json

.SUFFIXES: .c.o

.c.o:	$< $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@
