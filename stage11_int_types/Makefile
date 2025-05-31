
CC = gcc
CFLAGS = -Wall -g -std=gnu99 -Iinclude
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = $(BUILD_DIR)/mimic99

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

#mimic99: $(OBJS)
#	$(CC) $(OBJS) $(CFLAGS) -o mimic99

test: mimic99
	./run_all_tests.sh mimic99

clean:
	rm -rf $(BUILD_DIR)
	rm -f tests/*.s tests/*.o
	@for cfile in tests/test*.c; do \
		base=$${cfile%.c}; \
		if [ -f "$$base" ]; then \
			echo "Deleting binary $$base"; \
			rm -f "$$base"; \
		fi; \
	done

.PHONY: all clean	
