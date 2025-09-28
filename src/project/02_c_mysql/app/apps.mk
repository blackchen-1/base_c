# app/apps.mk
APP_DIR = app
SYSLOG_DIR = $(APP_DIR)/apps/syslog
APP_BUILD_DIR = $(BUILD_DIR)/apps

# 编译器设置
CC = gcc
CFLAGS += -Wall -O2 -I./common/mysql/include
LDFLAGS += -L$(BUILD_DIR)/lib
APP_LIBS = -lmysql_common -lmysqlclient -lpthread

# Syslog 应用设置
# SYSLOG_TARGET = $(APP_BUILD_DIR)/syslog_app
# SYSLOG_SOURCES = $(wildcard $(SYSLOG_DIR)/*.c)
# SYSLOG_OBJECTS = $(SYSLOG_SOURCES:$(SYSLOG_DIR)/%.c=$(APP_BUILD_DIR)/syslog/%.o)

# 主应用设置
MAIN_TARGET = $(APP_BUILD_DIR)/main_app
MAIN_SOURCES = $(APP_DIR)/main.c
MAIN_OBJECTS = $(APP_BUILD_DIR)/main.o

# .PHONY: app_syslog app_main app_clean app_install
.PHONY: app_main app_clean app_install

# app_syslog: $(SYSLOG_TARGET)

app_main: $(MAIN_TARGET)

# 构建所有应用
# app: app_syslog app_main
app: app_main

# Syslog 应用
# $(SYSLOG_TARGET): $(SYSLOG_OBJECTS) $(BUILD_DIR)/lib/libmysql_common.a
# 	@mkdir -p $(APP_BUILD_DIR)
# 	$(CC) -o $@ $(SYSLOG_OBJECTS) $(LDFLAGS) $(APP_LIBS)
# 	@echo "Syslog application $@ created."

# 主应用
$(MAIN_TARGET): $(MAIN_OBJECTS) $(BUILD_DIR)/lib/libmysql_common.a
	@mkdir -p $(APP_BUILD_DIR)
	$(CC) -o $@ $(MAIN_OBJECTS) $(LDFLAGS) $(APP_LIBS)
	@echo "Main application $@ created."

# 编译 syslog 对象文件
# $(APP_BUILD_DIR)/syslog/%.o: $(SYSLOG_DIR)/%.c
# 	@mkdir -p $(APP_BUILD_DIR)/syslog
# 	$(CC) $(CFLAGS) -c $< -o $@

# 编译主对象文件
$(APP_BUILD_DIR)/main.o: $(APP_DIR)/main.c
	@mkdir -p $(APP_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
app_clean:
	@rm -rf $(APP_BUILD_DIR)
	@echo "Applications clean completed."

# 安装
app_install:
	@echo "Installing applications..."
	@echo "Applications installed to $(APP_BUILD_DIR)"
