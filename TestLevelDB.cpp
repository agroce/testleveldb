#include <cassert>
#include "leveldb/db.h"

#include <deepstate/DeepState.hpp>

using namespace deepstate;

#define DATABASE_LOCATION "/Volumes/ramdisk/testdb"

TEST(LevelDB, Fuzz) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, DATABASE_LOCATION, &db);
  ASSERT(status.ok()) << "Could not create the test database!";

  system("rm -rf " DATABASE_LOCATION);
}
