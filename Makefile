
CC := g++ 

TARGET := bin/cremi
 
SRCEXT := cpp
SOURCES := base64.cpp remi.cpp websocket.cpp remi_server.cpp main.cpp
CFLAGS := -Wall -std=c++11
LIB := -pthread -lmicrohttpd
INC := 


all:
	$(CC) $(CFLAGS) $(SOURCES) $(LIB) -o $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)