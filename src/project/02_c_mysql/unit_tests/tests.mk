# unit_tests/tests.mk
TESTS_DIR = unit_tests
TESTS_BUILD_DIR = $(BUILD_DIR)/tests

# 编译器设置
CC = gcc
CFLAGS += -Wall -O2 -I./common/mysql/include -I./app
LDFLAGS += -L$(BUILD_DIR)/lib
TEST_LIBS = -lmysql_common -lmysqlclient -lpthread

# 测试文件设置
TEST_SOURCES = $(wildcard $(TESTS_DIR)/*.c)
TEST_TARGETS = $(TEST_SOURCES:$(TESTS_DIR)/%.c=$(TESTS_BUILD_DIR)/%)

.PHONY: unit_tests tests_clean tests_run

unit_tests: $(TEST_TARGETS)

# 编译测试程序
$(TESTS_BUILD_DIR)/%: $(TESTS_DIR)/%.c $(BUILD_DIR)/lib/libmysql_common.a
	@mkdir -p $(TESTS_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(TEST_LIBS)
	@echo "Test $@ created."

# 运行测试
tests_run: unit_tests
	@echo "Running unit tests..."
	@for test in $(TEST_TARGETS); do \
		if [ -f "$$test" ]; then \
			echo "Running $$test..."; \
			$$test || echo "Test $$test failed"; \
		fi \
	done
	@echo "All tests completed."

# 清理
tests_clean:
	@rm -rf $(TESTS_BUILD_DIR)
	@echo "Tests clean completed."
