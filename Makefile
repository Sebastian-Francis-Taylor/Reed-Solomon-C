CFLAGS = -Wall -O2 -I./src

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/galios.c $(SRC_DIR)/rs_decoder.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/rs_demo
TARGET_EXE = $(BIN_DIR)/rs_demo.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET) $(TARGET_EXE)

.PHONY: all build clean
