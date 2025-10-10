CC       = gcc
CFLAGS   = -Wall -Wextra -O$(OPT) $(shell pkg-config --cflags criterion) \
           -Wno-missing-field-initializers
INCLUDES = -Iecs/include -Iecs/ -Iecs/world -Iecs/datastructure -Idsl
LDLIBS   = $(shell pkg-config --libs criterion)

SRC      = $(wildcard *.c ecs/*.c ecs/world/*.c dsl/*.c ecs/addons/*.c)
OBJ      = $(patsubst %.c,build/%.o,$(SRC))

BIN      = build/main
TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(patsubst %.c,build/%.o,$(TEST_SRC))
TEST_BIN = build/tests_runner

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(BIN)
	./$(BIN)

TEST_OBJS = $(filter-out build/main.o,$(OBJ)) $(TEST_OBJ)

$(TEST_BIN): $(TEST_OBJS)
	@$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ $(LDLIBS)

test: $(TEST_BIN)
	./$(TEST_BIN) --verbose

debug: CFLAGS += -O0 -g -fsanitize=address,undefined
debug: LDLIBS += -fsanitize=address,undefined
debug: clean $(BIN)

debug-run: debug
	@./$(BIN)

debug-test: CFLAGS += -O0 -g -fsanitize=address,undefined
debug-test: LDLIBS += -fsanitize=address,undefined
debug-test: clean $(TEST_BIN)
	@./$(TEST_BIN) --verbose

perf: CFLAGS += -O2 -g
perf: clean $(BIN)
	xcrun xctrace record --template 'Time Profiler' \
		--output trace.trace \
		--launch -- ./$(BIN)

leak-check: CFLAGS += -O0 -g -fsanitize=address
leak-check: LDLIBS += -fsanitize=address
leak-check: clean $(BIN)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=0 ./$(BIN)

leak-check-apple: clean $(BIN)
	@leaks --atExit -- ./$(BIN)

clean:
	@rm -rf build

.PHONY: all clean run test debug debug-run debug-test perf leak-check leak-check-apple
