
CC = gcc
CFLAGS = -Wall -g -std=gnu99 -Iinclude
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
UNIT_TEST_DIR = unit_tests
UNIT_TEST_BUILD_DIR = unit_tests/build

SRCS = $(wildcard $(SRC_DIR)/*.c)
SRCS_NO_MAIN = $(filter-out $(SRC_DIR)/main.c, $(SRCS))
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
OBJS_NO_MAIN = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))
UNIT_TESTS = $(wildcard unit_tests/test_*.c)
UNIT_TEST_BINS = $(patsubst unit_tests/%.c, unit_tests/build/%, $(UNIT_TESTS))
TARGET = $(BUILD_DIR)/mimic99

all: $(TARGET)

unit_tests : $(UNIT_TEST_BINS)
	./run_unit_tests.sh

unit_tests/build/%: unit_tests/%.c $(OBJS_NO_MAIN)
	@mkdir -p $(UNIT_TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_NO_MAIN)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(TARGET)
	./run_all_tests.sh $(TARGET)

memcheck: $(TARGET)
	./run_all_tests.sh --memcheck $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(UNIT_TEST_BUILD_DIR);
	rm -f tests/*.s tests/*.o
	rm -f unit_tests/build/*.o
	@for cfile in tests/test*.c; do \
		base=$${cfile%.c}; \
		if [ -f "$$base" ]; then \
			echo "Deleting binary $$base"; \
			rm -f "$$base"; \
		fi; \
	done

.PHONY: all unit_tests clean
