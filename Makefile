CC = gcc
CFLAGS = -O2 -march=native -fno-strict-aliasing
LDFLAGS = -lm
TARGET = cache_latency
SOURCES = cache_latency.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS = -O0 -g -march=native -fno-strict-aliasing
debug: clean $(TARGET)

performance: CFLAGS = -O3 -march=native -fno-strict-aliasing -flto
performance: clean $(TARGET)

.PHONY: all clean run debug performance
