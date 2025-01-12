
CC = gcc  
CFLAGS = -Wall -Wextra -O2  

TARGET = user_program 
SRC = user_program.c  

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)  
clean:
	rm -f $(TARGET)  
