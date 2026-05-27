CXX := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++17 -O2 -g
TARGET := syscall-sentinel
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := test_programs
LOG_DIR := logs

SRCS := $(SRC_DIR)/main.cpp $(SRC_DIR)/monitor.cpp $(SRC_DIR)/rules.cpp $(SRC_DIR)/logger.cpp
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

TEST_SAFE := $(TEST_DIR)/safe_program
TEST_SUSPICIOUS := $(TEST_DIR)/suspicious_program
TEST_FORK := $(TEST_DIR)/fork_bomb_demo

.PHONY: all clean run-safe run-suspicious run-fork docker-build docker-run docker-shell

all: $(LOG_DIR) $(TARGET) $(TEST_SAFE) $(TEST_SUSPICIOUS) $(TEST_FORK)

$(LOG_DIR):
	mkdir -p $(LOG_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TEST_SAFE): $(TEST_DIR)/safe_program.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_SUSPICIOUS): $(TEST_DIR)/suspicious_program.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_FORK): $(TEST_DIR)/fork_bomb_demo.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

run-safe: all
	./$(TARGET) ./$(TEST_SAFE)

run-suspicious: all
	./$(TARGET) ./$(TEST_SUSPICIOUS)

run-fork: all
	./$(TARGET) ./$(TEST_FORK)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_SAFE) $(TEST_SUSPICIOUS) $(TEST_FORK) $(LOG_DIR)/sentinel.log

docker-build:
	docker build -t syscall-sentinel .

docker-run: docker-build
	docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined syscall-sentinel

docker-shell: docker-build
	docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "$$(pwd)/logs:/app/logs" syscall-sentinel bash
