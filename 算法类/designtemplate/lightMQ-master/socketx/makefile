DIR_SRC = ./src
DIR_INC = ./include
DIR_TEST = ./tests
DIR_EXP = ./examples
LIB_PATH ?= ./libs

SRC = $(wildcard ${DIR_SRC}/*.cpp)

VPATH = ./src
CFLAGS += -I$(DIR_INC) -std=c++11 -fPIC -pthread

SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})

.PHONY: all
all: $(LIB_PATH)/libsocketx.a
	cd examples && $(MAKE)

${DIR_SRC}/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

$(LIB_PATH):
	@mkdir -p libs

$(LIB_PATH)/libsocketx.a: $(SRC_OBJ) | $(LIB_PATH)
	ar rcs $@ $^

examples: 
	for dir in $(DIR_EXP); do\
		(cd $$dir; ${MAKE});\
	done

tests: 
	for dir in $(DIR_TEST); do\
		(cd $$dir; ${MAKE});\
	done

.PHONY: clean
clean:
	rm -f $(SRC_OBJ)
	rm -f $(LIB_PATH)/libsocketx.a
	for dir in $(DIR_EXP); do\
		(cd $$dir; ${MAKE} clean);\
	done
	for dir in $(DIR_TEST); do\
		(cd $$dir; ${MAKE} clean);\
	done
