CXX=/usr/local/opt/llvm\@6/bin/clang++
CC=/usr/local/opt/llvm\@6/bin/clang

LEVELDB=/Users/alex/leveldb

all: TestLevelDB TestLevelDB_LF

clean:
	rm TestLevelDB TestLevelDB_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=integer,undefined,address -ldeepstate

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,integer,undefined,address -ldeepstate_LF
