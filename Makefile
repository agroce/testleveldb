CC=clang
CXX=clang++

LEVELDB=/root/leveldb
ROCKSDB=/root/rocksdb

all: TestLevelDB TestLevelDB_LF

clean:
	rm TestLevelDB TestLevelDB_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=undefined,address -ldeepstate

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF
