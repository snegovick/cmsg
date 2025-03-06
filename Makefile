TESTS = test_basic_int_serdes test_basic_real_serdes test_basic_str_serdes test_hash_serdes test_nested_hash_serdes test_array_serdes test_json_parser
CROSS_COMPILE ?=
CROSS_ROOT ?=
PKG_CONFIG ?= pkg-config
PREFIX ?= .
BRANCH ?= master
VERSION = $(shell git describe)

ifeq ($(CROSS_COMPILE),)
CC ?= gcc
else
CC = $(CROSS_COMPILE)gcc
endif

CFLAGS ?=
ifneq ($(CROSS_ROOT),)
CFLAGS += -I$(CROSS_ROOT)/include
endif
CFLAGS_EXTRA = -Wall -Wextra -Wno-unused-function -Wno-unused-parameter
CFLAGS_EXTRA += -Wall -I./ 
CFLAGS_EXTRA += -ggdb3 -O0

LDFLAGS ?=
LDFLAGS_EXTRA = -ggdb3 -O0 -lcgreen
ifneq ($(CROSS_ROOT),)
LDFLAGS += -L$(CROSS_ROOT)/lib
endif
LDFLAGS_EXTRA += -lc -lm

BASIC_INT_SOURCES = test_basic_int_serdes.c cmsg.c
BASIC_INT_OBJECTS = $(patsubst %.c, %.o, $(BASIC_INT_SOURCES))

BASIC_REAL_SOURCES = test_basic_real_serdes.c cmsg.c
BASIC_REAL_OBJECTS = $(patsubst %.c, %.o, $(BASIC_REAL_SOURCES))

BASIC_STR_SOURCES = test_basic_str_serdes.c cmsg.c
BASIC_STR_OBJECTS = $(patsubst %.c, %.o, $(BASIC_STR_SOURCES))

TEST_HASH_SOURCES = test_hash_serdes.c cmsg.c
TEST_HASH_OBJECTS = $(patsubst %.c, %.o, $(TEST_HASH_SOURCES))

TEST_NESTED_HASH_SOURCES = test_nested_hash_serdes.c cmsg.c
TEST_NESTED_HASH_OBJECTS = $(patsubst %.c, %.o, $(TEST_NESTED_HASH_SOURCES))

TEST_ARRAY_SOURCES = test_array_serdes.c cmsg.c
TEST_ARRAY_OBJECTS = $(patsubst %.c, %.o, $(TEST_ARRAY_SOURCES))

TEST_JSON_SOURCES = test_json_parser.c cmsg.c cmj.c
TEST_JSON_OBJECTS = $(patsubst %.c, %.o, $(TEST_JSON_SOURCES))

all: $(TESTS)
	./test_basic_int_serdes
	./test_basic_real_serdes
	./test_basic_str_serdes
	./test_hash_serdes
	./test_nested_hash_serdes
	./test_array_serdes
	./test_json_parser

test_basic_int_serdes: $(BASIC_INT_OBJECTS)
	$(CC) $(BASIC_INT_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_basic_real_serdes: $(BASIC_REAL_OBJECTS)
	$(CC) $(BASIC_REAL_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_basic_str_serdes: $(BASIC_STR_OBJECTS)
	$(CC) $(BASIC_STR_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_hash_serdes: $(TEST_HASH_OBJECTS)
	$(CC) $(TEST_HASH_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_nested_hash_serdes: $(TEST_NESTED_HASH_OBJECTS)
	$(CC) $(TEST_NESTED_HASH_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_array_serdes: $(TEST_ARRAY_OBJECTS)
	$(CC) $(TEST_ARRAY_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

test_json_parser: $(TEST_JSON_OBJECTS)
	$(CC) $(TEST_JSON_OBJECTS) $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -c $< -o $@

# .PHONY: install
# install:
# ifneq ($(PREFIX),.)
# 	cp $(TOOL) $(PREFIX)
# endif

.PHONY: clean
clean:
	-rm $(TESTS) $(BASIC_INT_OBJECTS) $(BASIC_REAL_OBJECTS) $(BASIC_STR_OBJECTS) $(TEST_HASH_OBJECTS) $(TEST_NESTED_HASH_OBJECTS) $(TEST_ARRAY_OBJECTS) $(TEST_JSON_PARSER)
