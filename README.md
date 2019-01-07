1.  install DeepState
2.  install leveldb, after editing its CMakeLists.txt to add -fsanitize=integer,undefined,address,fuzzer-no-link
3.  edit Makefile to point to good clang and leveldb
3.  fuzz
