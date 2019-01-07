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

#define DATABASE_LOCATION "/Volumes/ramdisk/testdb"

#define TEST_LENGTH 10

#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 64

TEST(LevelDB, Fuzz) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, DATABASE_LOCATION, &db);
  ASSERT(status.ok()) << "Could not create the test database!";

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
	    leveldb::Status s = db->Put(write_options, key, value);
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
	    leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
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
	    leveldb::Status s = db->Delete(write_options, key);
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
	    leveldb::Status s = db->Write(leveldb::WriteOptions(), &batch);
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
  
  delete db;
  rmrf(DATABASE_LOCATION);
}
