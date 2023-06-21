
cc=gcc

DEPS=json-c libcurl

CFLAGS += $(shell pkg-config --cflags $(DEPS))
LDFLAGS += $(shell pkg-config --libs $(DEPS))

all:
	$(cc) $(CFLAGS) $(LDFLAGS) epic.c -o epic -lcurl
