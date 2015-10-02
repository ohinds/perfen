export BIN_DIR = $(PWD)/bin
export OBJ_DIR = $(PWD)/obj

export C_FLAGS = -std=c++11 \
                 -g \
                 -I $(PWD)/ext/yaml-cpp/include
export C_LIBS = `pkg-config --cflags --libs jack` \
                -lncurses \
                -lsndfile \
                $(PWD)/ext/yaml-cpp/build/libyaml-cpp.a

default: perfen

perfen:
	cd src && make
	g++ $(C_FLAGS) -o $(BIN_DIR)/perfen $(OBJ_DIR)/*.o $(C_LIBS)

yaml-cpp:
	mkdir -p ext
	cd ext; \
	git clone https://github.com/jbeder/yaml-cpp.git; \
	mkdir -p yaml-cpp/build; \
	cd yaml-cpp/build; \
	cmake ..; \
	make

clean:
	rm $(OBJ_DIR)/* $(BIN_DIR)/*
