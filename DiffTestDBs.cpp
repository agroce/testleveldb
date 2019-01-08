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

#define LEVELDB_LOCATION "/mnt/ramdisk/testleveldb"
#define ROCKSDB_LOCATION "/mnt/ramdisk/testrocksdb"

#define TEST_LENGTH 20

#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 64

TEST(LevelDB, Fuzz) {
  leveldb::DB* l_db;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_status = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT (l_status.ok()) << "Could not create the leveldb test database!";

  rocksdb::DB* r_db;
  rocksdb::Options r_options;
  r_options.create_if_missing = true;
  rocksdb::Status r_status = rocksdb::DB::Open(r_options, ROCKSDB_LOCATION, &r_db);
  ASSERT (r_status.ok()) << "Could not create the rocksdb test database!";

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
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }

	    rocksdb::WriteOptions r_write_options;
	    if (synced) {
	      r_write_options.sync = true;	    
	    }
	    rocksdb::Status r_s = r_db->Put(r_write_options, key, value);
	    if (!r_s.ok()) {
	      LOG(TRACE) << n << ": rocksdb NOT OK: " << r_s.ToString();
	      ASSERT (!l_s.ok()) << "Mismatch:  rocksb not ok, leveldb ok";
	    } else {
	      ASSERT (l_s.ok()) << "Mismatch:  rocksdb ok, leveldb not ok";	      
	    }
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
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << l_value;
	    } else {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }

	    std::string r_value;
	    rocksdb::Status r_s = r_db->Get(rocksdb::ReadOptions(), key, &r_value);
	    if (r_s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << r_value;
	      ASSERT (r_s.ok()) << "Mismatch:  rocksb ok, leveldb not ok";
	      ASSERT_EQ (l_value, r_value) << l_value << " SHOULD EQUAL " << r_value;
	    } else {
	      LOG(TRACE) << n << ": rocksdb NOT OK: " << l_s.ToString();
	      ASSERT (!l_s.ok()) << "Mismatch:  rocksb not ok, leveldb ok";
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
	    rocksdb::WriteOptions r_write_options;
	    if (synced) {
	      r_write_options.sync = true;	    
	    }	    
	    rocksdb::Status r_s = r_db->Delete(r_write_options, key);
	    if (!r_s.ok()) {
	      LOG(TRACE) << n << ": rocksdb NOT OK: " << r_s.ToString();
	      ASSERT (!l_s.ok()) << "Mismatch:  rocksb not ok, leveldb ok";
	    } else {
	      ASSERT (l_s.ok()) << "Mismatch:  rocksdb ok, leveldb not ok";	      
	    }
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
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }

	    rocksdb::Status r_s = r_db->Write(rocksdb::WriteOptions(), &r_batch);
	    if (!r_s.ok()) {
	      LOG(TRACE) << n << ": rocksdb NOT OK: " << r_s.ToString();
	      ASSERT (!l_s.ok()) << "Mismatch:  rocksb not ok, leveldb ok";
	    } else {
	      ASSERT (l_s.ok()) << "Mismatch:  rocksdb ok, leveldb not ok";	      
	    }
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";
	    
	    l_batch.Clear();
	    r_batch.Clear();
	  }
	  );
  }
  
  delete l_db;
  rmrf(LEVELDB_LOCATION);
  delete r_db;
  rmrf(ROCKSDB_LOCATION);
}
