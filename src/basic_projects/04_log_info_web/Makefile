CC = gcc
LIBS = -lmysqlclient -lpthread
TARGET = log_server
SOURCE = log_server.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(LIBS)

debug: $(SOURCE)
	$(CC) -g -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
