CC = clang
BUILD_DIR = build
SRC = $(wildcard src/*/*.c) $(wildcard src/*.c) $(wildcard deps/*/*.c)
CFLAGS = $(shell cat compile_flags.txt | tr '\n' ' ')
CFLAGS += -lcurl
TEST_DIR = test

all: build/doubly_linked_list

build/doubly_linked_list:
	mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/doubly_linked_list $(SRC) $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: doubly_linked_list_test
doubly_linked_list_test:
	$(CC) -o $(BUILD_DIR)/doubly_linked_list_test $(SRC) test/doubly_linked_list_test.c $(CFLAGS)

.PHONY: test
test: doubly_linked_list_test
	$(BUILD_DIR)/doubly_linked_list_test
