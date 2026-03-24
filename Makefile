# Настройки для кросс-компиляции на macOS
CC = clang
TARGET = mips-linux-gnu
CFLAGS = -target $(TARGET) -static -O2 -Wall

# Имя выходного файла
OUT = jerboa_gateway_mips

all:
	$(CC) $(CFLAGS) jerboa_gateway.c -o $(OUT)
	@echo "✅ Сборка для EdgeRouter X завершена: $(OUT)"

clean:
	rm -f $(OUT)