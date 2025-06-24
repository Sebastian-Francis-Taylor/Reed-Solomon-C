CFLAGS = -Wall -O2 -I./src

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/galios.c $(SRC_DIR)/rs_decoder.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/rs_demo

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
