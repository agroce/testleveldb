#include "Common.hpp"

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
  int rv = remove(fpath);
  if (rv)
    perror(fpath);
  return rv;
}

int rmrf(const char *path) {
  return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

char* GetKey() {
  return DeepState_CStrUpToLen(MAX_KEY_LENGTH, ALPHABET);
}

char* GetValue() {
  return DeepState_CStrUpToLen(MAX_VALUE_LENGTH, ALPHABET);
}
