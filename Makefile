CC = gcc
CFLAGS = -Wall -Wextra -O2

# Kaynak kodlarin dizini
SRC_DIR = src

# Olusturulacak calistirilabilir dosyanin adi
TARGET = tarsau

all: $(TARGET)

$(TARGET): $(SRC_DIR)/tarsau.c
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_DIR)/tarsau.c

clean:
	rm -f $(TARGET) a.sau
