LEVELDB=/root/leveldb

all: TestLevelDB TestLevelDB_LF

clean:
	rm TestLevelDB TestLevelDB_LF

TestLevelDB: TestLevelDB.cpp
	$(CXX) -o TestLevelDB TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=undefined,address -ldeepstate -std=c++11

TestLevelDB_LF: TestLevelDB.cpp
	$(CXX) -o TestLevelDB_LF TestLevelDB.cpp -I$(LEVELDB)/include $(LEVELDB)/build/libleveldb.a -fsanitize=fuzzer,undefined,address -ldeepstate_LF -std=c++11
