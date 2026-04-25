# ================================================================
# Autopark — Makefile
# Team: Короткевич Дмитрий (TL) & Близник Климентий (Dev)
# ================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -Iincludes -fprofile-arcs -ftest-coverage
LDFLAGS = -lsqlite3

SRC_DIR   = src
BLD_DIR   = build
BIN_DIR   = bin
TEST_DIR  = tests

SRCS    = $(SRC_DIR)/main.c    \
          $(SRC_DIR)/db.c      \
          $(SRC_DIR)/auth.c    \
          $(SRC_DIR)/cars.c    \
          $(SRC_DIR)/drivers.c \
          $(SRC_DIR)/orders.c

LIB_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(LIB_SRCS))
MAIN_OBJ = $(BLD_DIR)/main.o

TARGET  = $(BIN_DIR)/autopark

# Test binaries
TEST_CARS    = $(BIN_DIR)/test_cars
TEST_DRIVERS = $(BIN_DIR)/test_drivers
TEST_ORDERS  = $(BIN_DIR)/test_orders

.PHONY: all clean check distcheck

# ── default ──────────────────────────────────────────────────────
all: $(TARGET)

$(BLD_DIR):
	mkdir -p $(BLD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c | $(BLD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(LIB_OBJS) $(MAIN_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# ── test builds ──────────────────────────────────────────────────
$(TEST_CARS): $(LIB_OBJS) $(TEST_DIR)/test_cars.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_DRIVERS): $(LIB_OBJS) $(TEST_DIR)/test_drivers.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_ORDERS): $(LIB_OBJS) $(TEST_DIR)/test_orders.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# ── check (run all tests) ─────────────────────────────────────────
check: $(TEST_CARS) $(TEST_DRIVERS) $(TEST_ORDERS)
	@echo "--- Running unit tests ---"
	$(TEST_CARS)
	$(TEST_DRIVERS)
	$(TEST_ORDERS)
	@echo "--- All tests completed ---"

# ── distcheck (tests + coverage report) ──────────────────────────
distcheck: check
	@echo "--- Generating coverage report ---"
	gcov -o $(BLD_DIR) $(LIB_SRCS) 2>/dev/null || true
	@echo "--- Coverage done ---"

# ── clean ────────────────────────────────────────────────────────
clean:
	rm -rf $(BLD_DIR) $(BIN_DIR) *.gcov *.gcda *.gcno
