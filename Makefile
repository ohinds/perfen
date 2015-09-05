export BIN_DIR = $(PWD)/bin
export OBJ_DIR = $(PWD)/obj

export CC = g++
export C_FLAGS = -std=c++11 -g
export C_LIBS = `pkg-config --cflags --libs jack` \
                -lncurses \
                -lyaml-cpp

default: perfen

perfen:
	cd src && make
	$(CC) $(C_FLAGS) -o $(BIN_DIR)/perfen $(OBJ_DIR)/*.o $(C_LIBS)

clean:
	rm $(OBJ_DIR)/* $(BIN_DIR)/*
