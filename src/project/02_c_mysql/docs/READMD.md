.
├── Makefile
├── common/                    # 通用组件
│   ├── mysql/                # MySQL通用接口
│   │   ├── include/
│   │   │   └── mysql_common_sql.h
│   │   ├── src/
│   │   │   └── mysql_common_sql.c
│   │   └── Makefile
│   └── utils/                # 通用工具函数
├── projects/                  # 项目封装层
│   ├── syslog/
│   │   ├── include/
│   │   ├── src/
│   │   ├── examples/
│   │   └── Makefile
│   ├── user_management/
│   └── product_catalog/
├── tests/                     # 测试代码
│   ├── unit_tests/
│   └── integration_tests/
├── docs/                      # 文档
└── build/                     # 构建输出
