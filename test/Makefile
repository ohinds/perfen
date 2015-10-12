TEST_OBJ_DIR = obj

.DELETE_ON_ERROR:

.SUFFIXES:
.SUFFIXES:  .o .cpp

# suffix rule for subsidiary source files
# (see http://www.gnu.org/manual/make/html_chapter/make_10.html#SEC111)
$(TEST_OBJ_DIR)/%.o: %.cpp
	@echo '[make: building $@]'
	g++ -I../src $(C_FLAGS) -o $@ -c $<

SRC_FILES = $(wildcard ./*.cpp)
TMP_FILES = $(patsubst ./%,$(TEST_OBJ_DIR)/%,$(SRC_FILES))
OBJ_FILES = $(TMP_FILES:.cpp=.o)

TEST_LIBS = -lpthread /usr/lib/gtest_main.a

default: run_tests

run_tests: test_bin
	$(BIN_DIR)/run_tests

test_bin: $(OBJ_FILES)
	$(CXX) -o $(BIN_DIR)/run_tests $(OBJ_FILES) $(OBJ_DIR)/*.o $(C_LIBS) $(TEST_LIBS)
