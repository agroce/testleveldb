#ifndef COMMON_HPP

#include <ftw.h>

#include <deepstate/DeepState.hpp>
using namespace deepstate;

#define LEVELDB_LOCATION "testleveldb"
#define ROCKSDB_LOCATION "testrocksdb"

#define TEST_LENGTH 50

#define MAX_KEY_LENGTH 32
#define MAX_VALUE_LENGTH 128

// define as 0 for arbitrary bytestrings
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int rmrf(const char *path);

char* GetKey();
char* GetValue();

#endif
