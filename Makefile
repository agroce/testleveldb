CC=clang
CXX=clang++

LEVELDB=/root/leveldb
ROCKSDB=/root/rocksdb

all: DiffTestDBs DiffTestDBs_LF

clean:
	rm -rf DiffTestDBs DiffTestDBs_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=undefined,address -ldeepstate

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF

DiffTestDBs: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs DiffTestDBs.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=undefined,address -ldeepstate

DiffTestDBs_LF: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs_LF DiffTestDBs.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF
