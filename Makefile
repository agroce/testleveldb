LEVELDB=~/deepstate_leveldb
ROCKSDB=~/deepstate_rocksdb

all: TestLevelDB TestLevelDB_LF DiffTestDBs DiffTestDBs_LF

clean:
	rm -rf TestLevelDB TestLevelDB_LF DiffTestDBs DiffTestDBs_LF Te

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp Common.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -ldeepstate -lsnappy -lpthread

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp Common.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -lsnappy -lpthread -ldeepstate_LF

DiffTestDBs: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs DiffTestDBs.cpp Common.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -ldeepstate -lpthread -lz -lsnappy -llz4 -lbz2 -lzstd

DiffTestDBs_LF: DiffTestDBs.cpp
	$(CXX) -o DiffTestDBs_LF DiffTestDBs.cpp Common.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -I$(ROCKSDB)/include $(ROCKSDB)/librocksdb.a -fsanitize=fuzzer,undefined,address -lpthread -lz -lsnappy -llz4 -lbz2 -lzstd -ldeepstate_LF
