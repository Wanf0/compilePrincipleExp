# 编译器和选项
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -Iinclude

# 目标可执行文件
TARGET = parser

# 源文件
SOURCES = src/main.cpp src/lexer.cpp src/error.cpp src/token.cpp src/parser.cpp src/semantic.cpp

# 头文件
HEADERS = include/lexer.h include/error.h include/token.h include/parser.h include/ast.h include/semantic.h

# 目标对象文件
OBJECTS = $(SOURCES:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译规则
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJECTS) $(TARGET)

# 运行单个测试
run: $(TARGET)
	./$(TARGET) sample/test01_basic.sy

# 运行所有测试
runall: $(TARGET)
	@for f in sample/*.sy; do \
		echo "Running $$f ..."; \
		./$(TARGET) $$f; \
		echo "--------------------------------"; \
	done

.PHONY: all clean run runall

