# 定义C语言编译器
CC = gcc

# 定义编译标志
CFLAGS = -g -Wall -std=c99

# 默认规则
all: bin/bot bin/bot2

# 生成 bin/bot
bin/bot: src/ex9q1.c src/ex9q1.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/bot src/ex9q1.c

# 生成 bin/bot2
bin/bot2: src/ex10q1.c src/ex10q1.h src/ex9q1.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/bot2 src/ex10q1.c

# 清理规则
clean:
	rm -f bin/bot bin/bot2
