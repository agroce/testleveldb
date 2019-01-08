#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#ifdef ROCKS_TOO
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/write_batch.h"
#endif

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
  ASSERT (l_status.ok()) << "Could not create the leveldb test database!";

#ifdef ROCKS_TOO
  rocksdb::DB* r_db;
  rocksdb::Options r_options;
  r_options.create_if_missing = true;
  rocksdb::Status r_status = rocksdb::DB::Open(r_options, ROCKSDB_LOCATION, &r_db);
  ASSERT (r_status.ok()) << "Could not create the rocksdb test database!";
#endif

  leveldb::WriteBatch batch;
  bool synced = false;
  
  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    char* value = DeepState_CStrUpToLen(MAX_VALUE_LENGTH);
	    synced = DeepState_Bool();	    
	    LOG(TRACE) << n << ": PUT " << key << " " << value << " " << synced;

	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;	    
	    }
	    leveldb::Status l_s = l_db->Put(l_write_options, key, value);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }

#ifdef ROCKS_TOO
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
#endif
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
	    leveldb::Status l_s = l_db->Get(leveldb::ReadOptions(), key, &value);
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << value;
	    } else {
	      LOG(TRACE) << n << ": NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": DELETE " << key;
	    leveldb::WriteOptions l_write_options;
	    if (DeepState_Bool()) {
	      l_write_options.sync = true;	    
	      LOG(TRACE) << n << ": sync = true";
	    }	    
	    leveldb::Status l_s = l_db->Delete(l_write_options, key);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = DeepState_CStrUpToLen(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": BATCH DELETE " << key;
	    batch.Delete(key);
	  },	  
	  [&] {
	    LOG(TRACE) << n << ": BATCH WRITE";
	    leveldb::Status l_s = l_db->Write(leveldb::WriteOptions(), &batch);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << l_s.ToString();
	    }	    
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";	    
	    batch.Clear();
	  }
	  );
  }
  
  delete l_db;
  rmrf(LEVELDB_LOCATION);

#ifdef ROCKS_TOO
  delete r_db;
  rmrf(ROCKSDB_LOCATION);
#endif
}
