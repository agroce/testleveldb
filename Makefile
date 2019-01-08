CC=/usr/local/opt/llvm\@6/bin/clang
CXX=/usr/local/opt/llvm\@6/bin/clang++

LEVELDB=/Users/alex/leveldb
ROCKSDB=/Users/alex/rocksdb

all: TestLevelDB TestLevelDB_LF TestBoth TestBoth_LF

clean:
	rm -rf TestLevelDB TestLevelDB_LF TestBoth TestBoth_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=undefined,address -ldeepstate

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF

TestBoth: TestLevelDB.cpp
	$(CXX) -o TestBoth TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=undefined,address -ldeepstate -DROCKS_TOO

TestBoth_LF: TestLevelDB.cpp
	$(CXX) -o TestBoth_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF -DROCKS_TOO
