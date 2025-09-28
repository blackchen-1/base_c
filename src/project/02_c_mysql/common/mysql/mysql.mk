# common/mysql/mysql.mk
COMMON_MYSQL_DIR = common/mysql
COMMON_MYSQL_INC_DIR = $(COMMON_MYSQL_DIR)/include
COMMON_MYSQL_SRC_DIR = $(COMMON_MYSQL_DIR)/src
COMMON_MYSQL_BUILD_DIR = $(BUILD_DIR)/common/mysql

# 编译器和链接器设置
CC = gcc
AR = ar
CFLAGS += -Wall -O2 -fPIC -I$(COMMON_MYSQL_INC_DIR) -I/usr/include/mysql
LDFLAGS += -L/usr/lib/mysql
LIBS += -lmysqlclient -lpthread

# 文件设置
COMMON_MYSQL_LIB_NAME = libmysql_common.a
COMMON_MYSQL_SHARED_NAME = libmysql_common.so
COMMON_MYSQL_SOURCES = $(wildcard $(COMMON_MYSQL_SRC_DIR)/*.c)
COMMON_MYSQL_OBJECTS = $(COMMON_MYSQL_SOURCES:$(COMMON_MYSQL_SRC_DIR)/%.c=$(COMMON_MYSQL_BUILD_DIR)/%.o)
COMMON_MYSQL_HEADERS = $(wildcard $(COMMON_MYSQL_INC_DIR)/*.h)

# 安装路径
INC_INSTALL_DIR = /usr/local/include
LIB_INSTALL_DIR = /usr/local/lib

# 目标
.PHONY: common_mysql common_mysql_static common_mysql_shared common_mysql_clean common_mysql_install common_mysql_system_install

# 默认目标：构建静态库和动态库
common_mysql: common_mysql_static common_mysql_shared

# 静态库
common_mysql_static: $(BUILD_DIR)/lib/$(COMMON_MYSQL_LIB_NAME)

$(BUILD_DIR)/lib/$(COMMON_MYSQL_LIB_NAME): $(COMMON_MYSQL_OBJECTS) | $(BUILD_DIR)/lib
	$(AR) rcs $@ $^
	@echo "Static library $@ created."

# 动态库
common_mysql_shared: $(BUILD_DIR)/lib/$(COMMON_MYSQL_SHARED_NAME)

$(BUILD_DIR)/lib/$(COMMON_MYSQL_SHARED_NAME): $(COMMON_MYSQL_OBJECTS) | $(BUILD_DIR)/lib
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo "Shared library $@ created."

# 创建目录的规则
$(BUILD_DIR)/lib:
	@mkdir -p $@

$(COMMON_MYSQL_BUILD_DIR):
	@mkdir -p $@

# 编译对象文件
$(COMMON_MYSQL_BUILD_DIR)/%.o: $(COMMON_MYSQL_SRC_DIR)/%.c | $(COMMON_MYSQL_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
common_mysql_clean:
	@rm -rf $(COMMON_MYSQL_BUILD_DIR)
	@rm -f $(BUILD_DIR)/lib/$(COMMON_MYSQL_LIB_NAME)
	@rm -f $(BUILD_DIR)/lib/$(COMMON_MYSQL_SHARED_NAME)
	@echo "MySQL common clean completed."

# 安装到构建目录（供其他模块使用）
common_mysql_install: common_mysql
	@mkdir -p $(BUILD_DIR)/include/mysql_common
	@cp $(COMMON_MYSQL_HEADERS) $(BUILD_DIR)/include/mysql_common/
	@echo "MySQL common library installed to build directory"

# 安装到系统目录（需要 sudo 权限）
common_mysql_system_install: common_mysql
	@sudo mkdir -p $(INC_INSTALL_DIR)/mysql_common
	@sudo mkdir -p $(LIB_INSTALL_DIR)
	@sudo cp $(COMMON_MYSQL_HEADERS) $(INC_INSTALL_DIR)/mysql_common/
	@sudo cp $(BUILD_DIR)/lib/$(COMMON_MYSQL_LIB_NAME) $(LIB_INSTALL_DIR)/
	@sudo cp $(BUILD_DIR)/lib/$(COMMON_MYSQL_SHARED_NAME) $(LIB_INSTALL_DIR)/
	@sudo ldconfig
	@echo "MySQL common library installed to system directories"
	@echo "Headers installed to $(INC_INSTALL_DIR)/mysql_common/"
	@echo "Libraries installed to $(LIB_INSTALL_DIR)/"

# 依赖关系
$(COMMON_MYSQL_OBJECTS): $(COMMON_MYSQL_HEADERS)
