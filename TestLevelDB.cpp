#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "ftw.h"

#include <deepstate/DeepState.hpp>

using namespace deepstate;

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
  int rv = remove(fpath);
  if (rv)
    perror(fpath);
  return rv;
}

int rmrf(const char *path) {
  return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

#define LEVELDB_LOCATION "/tmp/testleveldb"
#define ROCKSDB_LOCATION "/tmp/testrocksdb"

#define TEST_LENGTH 20

#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 64

TEST(LevelDB, Fuzz) {
  rmrf(LEVELDB_LOCATION);
  
  leveldb::DB* l_db;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_status = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT (l_status.ok()) << "Could not create the leveldb test database!";
  leveldb::WriteBatch l_batch;

  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);
	    bool synced = DeepState_Bool();	    
	    LOG(TRACE) << n << ": PUT " << key << " " << value << " " << synced;

	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;	    
	    }
	    leveldb::Status l_s = l_db->Put(l_write_options, key, value);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);

	    LOG(TRACE) << n << ": BATCH PUT " << key << " " << value;
	    
	    l_batch.Put(key, value);
	  },	  
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": GET " << key;

	    std::string l_value;
	    leveldb::Status l_s = l_db->Get(leveldb::ReadOptions(), key, &l_value);
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << l_value;
	    } else {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    bool synced = DeepState_Bool();
	    LOG(TRACE) << n << ": DELETE " << key << " " << synced;

	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;	    
	    }	    
	    leveldb::Status l_s = l_db->Delete(l_write_options, key);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": BATCH DELETE " << key;
	    
	    l_batch.Delete(key);
	  },	  
	  [&] {
	    LOG(TRACE) << n << ": BATCH WRITE";

	    leveldb::Status l_s = l_db->Write(leveldb::WriteOptions(), &l_batch);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";
	    
	    l_batch.Clear();
	  }
	  );
  }
  
  delete l_db;
}
