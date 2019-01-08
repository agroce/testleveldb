#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "rocksdb/db.h"
#include "rocksdb/options.h"
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

void check_status(int n, leveldb::Status l_s, rocksdb::Status r_s) {
  LOG(TRACE) << n << ": leveldb STATUS: " << l_s.ToString();
  LOG(TRACE) << n << ": rocksdb STATUS: " << r_s.ToString();
  ASSERT ((l_s.ok() && r_s.ok()) || ((!l_s.ok()) && (!r_s.ok()))) << "Status mismatch: " << l_s.ToString() <<
    " vs. " << r_s.ToString();
}

#define LEVELDB_LOCATION "/mnt/ramdisk/testleveldb"
#define ROCKSDB_LOCATION "/mnt/ramdisk/testrocksdb"

#define TEST_LENGTH 50

#define MAX_KEY_LENGTH 32
#define MAX_VALUE_LENGTH 128

TEST(LevelDB, Fuzz) {
  rmrf(LEVELDB_LOCATION);
  rmrf(ROCKSDB_LOCATION);
  
  leveldb::DB* l_db;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_s = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT (l_s.ok()) << "Could not create the leveldb test database!";

  rocksdb::DB* r_db;
  rocksdb::Options r_options;
  r_options.create_if_missing = true;
  rocksdb::Status r_s = rocksdb::DB::Open(r_options, ROCKSDB_LOCATION, &r_db);
  ASSERT (r_s.ok()) << "Could not create the rocksdb test database!";

  leveldb::WriteBatch l_batch;
  rocksdb::WriteBatch r_batch;
  
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

	    rocksdb::WriteOptions r_write_options;
	    if (synced) {
	      r_write_options.sync = true;	    
	    }
	    rocksdb::Status r_s = r_db->Put(r_write_options, key, value);

	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);

	    LOG(TRACE) << n << ": BATCH PUT " << key << " " << value;
	    
	    l_batch.Put(key, value);
	    r_batch.Put(key, value);
	  },	  
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": GET " << key;

	    std::string l_value;
	    leveldb::Status l_s = l_db->Get(leveldb::ReadOptions(), key, &l_value);

	    std::string r_value;
	    rocksdb::Status r_s = r_db->Get(rocksdb::ReadOptions(), key, &r_value);

	    check_status(n, l_s, r_s);
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT: " << l_value;
	    }
	    ASSERT_EQ (l_value, r_value) << l_value << " SHOULD EQUAL " << r_value;
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

	    rocksdb::WriteOptions r_write_options;
	    if (synced) {
	      r_write_options.sync = true;	    
	    }	    
	    rocksdb::Status r_s = r_db->Delete(r_write_options, key);

	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": BATCH DELETE " << key;
	    
	    l_batch.Delete(key);
	    r_batch.Delete(key);
	  },	  
	  [&] {
	    LOG(TRACE) << n << ": BATCH WRITE";

	    leveldb::Status l_s = l_db->Write(leveldb::WriteOptions(), &l_batch);
	    rocksdb::Status r_s = r_db->Write(rocksdb::WriteOptions(), &r_batch);
	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";
	    
	    l_batch.Clear();
	    r_batch.Clear();
	  }
	  );
  }
  
  delete l_db;
  delete r_db;
}
