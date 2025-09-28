# 顶层 Makefile
PROJECT_NAME = mysql_project
BUILD_DIR = build
export BUILD_DIR

# 包含各个模块的 Makefile 片段
include common/mysql/mysql.mk
include app/apps.mk
-include unit_tests/tests.mk  # 使用 -include 避免文件不存在时报错

# 默认目标
.PHONY: all clean build test install help run

all: build

build: common_mysql app

test:
	@echo "Unit tests not implemented yet"

install: common_mysql_install

# 直接运行主应用（设置库路径）
run: build
	@echo "Running main application..."
	@LD_LIBRARY_PATH=$(BUILD_DIR)/lib:$$LD_LIBRARY_PATH $(BUILD_DIR)/apps/main_app

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed."

help:
	@echo "Available targets:"
	@echo "  all          - Build all components"
	@echo "  build        - Build all components"
	@echo "  common_mysql - Build MySQL common library"
	@echo "  app          - Build applications"
	@echo "  run          - Run main application"
	@echo "  test         - Run unit tests"
	@echo "  clean        - Clean build files"
	@echo "  install      - Install components"
	@echo "  help         - Show this help"
