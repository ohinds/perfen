export BIN_DIR = $(PWD)/bin
export OBJ_DIR = $(PWD)/obj

export CC = g++
export C_FLAGS = -std=c++11 -g
export C_LIBS = `pkg-config --cflags --libs jack` -lncurses

default: perfen

perfen:
	cd src && make

clean:
	rm $(OBJ_DIR)/* $(BIN_DIR)/*
