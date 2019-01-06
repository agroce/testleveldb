#include <cassert>
#include "leveldb/db.h"
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

#define MAX_KEY_LENGTH 10
#define MAX_VALUE_LENGTH 10

TEST(LevelDB, Fuzz) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, DATABASE_LOCATION, &db);
  ASSERT(status.ok()) << "Could not create the test database!";

  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = DeepState_CStr(MAX_KEY_LENGTH);
	    char* value = DeepState_CStr(MAX_VALUE_LENGTH);
	    LOG(TRACE) << n << ": PUT " << key << " " << value;
	    leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
	    if (!s.ok()) {
	      LOG(TRACE) << n << ": NOT OK: " << s.ToString();
	    }
	  },
	  [&] {
	    std::string value;
	    char* key = DeepState_CStr(MAX_KEY_LENGTH);
	    LOG(TRACE) << n << ": GET " << key;
	    leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
	    if (s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << value;
	    } else {
	      LOG(TRACE) << n << ": NOT OK: " <<s.ToString();
	    }
	  }
	  );
  }
  
  delete db;
  rmrf(DATABASE_LOCATION);
}
