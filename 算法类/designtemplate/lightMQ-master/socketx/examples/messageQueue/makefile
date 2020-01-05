DIR_SRC = .
HEADER_PATH = ../../../socketx/include
LIB_SOCKETX_PATH = ../../../socketx/libs
SRC = $(wildcard ${DIR_SRC}/*.cpp)

VPATH = $(DIR_SRC)
CFLAGS += -I$(HEADER_PATH) -std=c++11 -pthread -g

SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})

	
messageQueue: ${SRC_OBJ}
	g++ $(CFLAGS) $< -o $@ -L$(LIB_SOCKETX_PATH) -lsocketx


./%.o:./%.cpp
	g++ $(CFLAGS) -c $< -o $@

${DIR_SRC}/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) ./*.o messageQueue
