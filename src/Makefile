.DELETE_ON_ERROR:

.SUFFIXES:
.SUFFIXES:  .o .cpp

# suffix rule for subsidiary source files
# (see http://www.gnu.org/manual/make/html_chapter/make_10.html#SEC111)
$(OBJ_DIR)/%.o: %.cpp %.h
	@echo '[make: building $@]'
	g++ $(C_FLAGS) -o $@ -c $<

HDR_FILES = $(wildcard *.h)
ALL_SRC_FILES = $(wildcard ./*.cpp)
SRC_FILES = $(filter-out ./main.cpp, $(ALL_SRC_FILES))
TMP_FILES = $(patsubst ./%,$(OBJ_DIR)/%,$(SRC_FILES))
OBJ_FILES = $(TMP_FILES:.cpp=.o)

default: $(OBJ_FILES)
