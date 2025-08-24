CFLAGS = -Wall -O2 -I./src
DEBUG_CFLAGS = -Wall -g -O0 -DDEBUG -I./src

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/galois.c $(SRC_DIR)/rs_decoder.c $(SRC_DIR)/rs_encoder.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEBUG_OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_debug.o)
TARGET = $(BIN_DIR)/rs_demo
TARGET_EXE = $(BIN_DIR)/rs_demo.exe
DEBUG_TARGET = $(BIN_DIR)/rs_demo_debug

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(DEBUG_CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_debug.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

valgrind: debug
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $(DEBUG_TARGET)

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*_debug.o $(TARGET) $(TARGET_EXE) $(DEBUG_TARGET)

.PHONY: all debug clean valgrind
