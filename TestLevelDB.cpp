#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "rocksdb/db.h"
#include "rocksdb/write_batch.h"
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
  leveldb::DB* l_db;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_status = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT(l_status.ok()) << "Could not create the leveldb test database!";

  rocksdb::DB* r_db;
  rocksdb::Options r_options;
  r_options.create_if_missing = true;
  rocksdb::Status r_status = rocksdb::DB::Open(r_options, ROCKSDB_LOCATION, &r_db);
  ASSERT(r_status.ok()) << "Could not create the rocksdb test database!";  

  leveldb::WriteBatch batch;
  
  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);
	    LOG(TRACE) << n << ": PUT " << key << " " << value;
	    leveldb::WriteOptions write_options;
	    if (DeepState_Bool()) {
	      write_options.sync = true;	    
	      LOG(TRACE) << n << ": sync = true";
	    }
	    leveldb::Status s = l_db->Put(write_options, key, value);
	    if (!s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);
	    LOG(TRACE) << n << ": BATCH PUT " << key << " " << value;
	    batch.Put(key, value);
	  },	  
	  [&] {
	    std::string value;
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": GET " << key;
	    leveldb::Status s = l_db->Get(leveldb::ReadOptions(), key, &value);
	    if (s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << value;
	    } else {
	      LOG(TRACE) << n << ": NOT OK: " <<s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": DELETE " << key;
	    leveldb::WriteOptions write_options;
	    if (DeepState_Bool()) {
	      write_options.sync = true;	    
	      LOG(TRACE) << n << ": sync = true";
	    }	    
	    leveldb::Status s = l_db->Delete(write_options, key);
	    if (!s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": BATCH DELETE " << key;
	    batch.Delete(key);
	  },	  
	  [&] {
	    LOG(TRACE) << n << ": BATCH WRITE";
	    leveldb::Status s = l_db->Write(leveldb::WriteOptions(), &batch);
	    if (!s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << s.ToString();
	    }	    
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";	    
	    batch.Clear();
	  }
	  );
  }
  
  delete l_db;
  delete r_db;
  rmrf(LEVELDB_LOCATION);
  rmrf(ROCKSDB_LOCATION);
}
