
CC := g++ 

TARGET := bin/cremi
 
SRCEXT := cpp
SOURCES := remi.cpp websocket.cpp remi_server.cpp main.cpp
CFLAGS := -Wall -std=c++11
LIB := -pthread -lssl -lcrypto -lmicrohttpd
INC := 


all:
	$(CC) $(CFLAGS) $(SOURCES) $(LIB) -o $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)