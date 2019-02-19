CC=~/afl/afl-clang
CXX=~/afl/afl-clang++

LEVELDB=/home/vagrant/leveldb
ROCKSDB=/home/vagrant/rocksdb

all: DiffTestDBs DiffTestDBs_LF

clean:
	rm -rf DiffTestDBs DiffTestDBs_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -ldeepstate -lsnappy -lpthread

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF

DiffTestDBs: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs DiffTestDBs.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -ldeepstate -lpthread -lz -lsnappy -llz4 -lbz2 -lzstd

DiffTestDBs_LF: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs_LF DiffTestDBs.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF
